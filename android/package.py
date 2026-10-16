#!/usr/bin/env python3
"""Package libspectrum.so into an installable, debug-signed APK. No Gradle.

    python android/package.py            # builds projects/android/<mode>/spectrum.apk
    python android/package.py --install  # ...then `adb install -r` it to the connected device

Steps: javac -> d8 (classes.dex) -> aapt2 link (manifest) -> add dex + lib/arm64-v8a/*.so
-> zipalign -> apksigner (debug keystore, created on first use).
"""
import argparse
import os
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
APP = ROOT / "android" / "app"
OUT = ROOT / "projects" / "android"
SHADER_CACHE = OUT / "shader_cache" / "vulkan"
ENGINE_ASSETS = OUT / "shader_cache" / "assets"

SDK = next(Path(p) for p in (os.environ.get("ANDROID_HOME"), os.environ.get("ANDROID_SDK_ROOT"),
                             os.path.join(os.environ.get("LOCALAPPDATA", ""), "Android", "Sdk"))
           if p and Path(p).is_dir())
BUILD_TOOLS = max((d for d in (SDK / "build-tools").iterdir() if d.is_dir()),
                  key=lambda d: [int(x) for x in d.name.split(".") if x.isdigit()])
PLATFORM = max((d for d in (SDK / "platforms").iterdir() if (d / "android.jar").exists()),
               key=lambda d: [int(x) for x in d.name.replace("android-", "").split(".") if x.isdigit()])
ANDROID_JAR = PLATFORM / "android.jar"
NDK = max((d for d in (SDK / "ndk").iterdir() if d.is_dir()), key=lambda d: [int(x) for x in d.name.split(".") if x.isdigit()])
NDK_STRIP = NDK / "toolchains" / "llvm" / "prebuilt" / "windows-x86_64" / "bin" / "llvm-strip.exe"


def java_tool(name):
    for base in (os.environ.get("JAVA_HOME"), r"C:\Program Files\Java\latest", r"C:\Program Files\Android\Android Studio\jbr"):
        if base and (Path(base) / "bin" / f"{name}.exe").exists():
            return Path(base) / "bin" / f"{name}.exe"
    for d in sorted(Path(r"C:\Program Files\Java").glob("jdk*"), reverse=True):
        if (d / "bin" / f"{name}.exe").exists():
            return d / "bin" / f"{name}.exe"
    sys.exit(f"{name}.exe not found (set JAVA_HOME)")


def run(cmd, **kw):
    cmd = [str(c) for c in cmd]
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        sys.exit(f"command failed ({r.returncode}): {' '.join(cmd)}\n{r.stdout}{r.stderr}")
    return r


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mode", default="debug", choices=["debug", "profile", "retail"])
    ap.add_argument("--install", action="store_true")
    args = ap.parse_args()

    so = OUT / args.mode / "lib" / "libspectrum.so"
    if not so.exists():
        sys.exit(f"{so} missing: run  python android/build.py --target app  first")

    work = OUT / args.mode / "apk"
    shutil.rmtree(work, ignore_errors=True)
    (work / "classes").mkdir(parents=True)

    # The debug mode is built with full -g; the device does not need it (android/debug.py reads the
    # unstripped library from lib/, matching by build-id), so the APK gets a copy without DWARF.
    device_lib = so
    if args.mode == "debug":
        device_lib = work / "libspectrum.so"
        run([NDK_STRIP, "--strip-debug", "-o", device_lib, so])

    # 1. Java -> dex
    run([java_tool("javac"), "--release", "8", "-Xlint:-options", "-classpath", ANDROID_JAR,
         "-d", work / "classes", APP / "MainActivity.java"])
    class_files = [str(p) for p in (work / "classes").rglob("*.class")]
    # d8.bat passes -Djava.ext.dirs, which JDK 9+ refuses to start with.
    run([java_tool("java"), "-cp", BUILD_TOOLS / "lib" / "d8.jar", "com.android.tools.r8.D8",
         "--lib", ANDROID_JAR, "--min-api", "29", "--output", work, *class_files])

    # 2. manifest -> base apk
    base = work / "base.apk"
    run([BUILD_TOOLS / "aapt2.exe", "link", "-o", base, "-I", ANDROID_JAR, "--manifest", APP / "AndroidManifest.xml",
         "--min-sdk-version", "29", "--target-sdk-version", "35"])

    # 3. add classes.dex and the native library
    unaligned = work / "unaligned.apk"
    with zipfile.ZipFile(base) as src, zipfile.ZipFile(unaligned, "w") as dst:
        for item in src.infolist():
            dst.writestr(item, src.read(item.filename))
        dst.write(work / "classes.dex", "classes.dex", compress_type=zipfile.ZIP_DEFLATED)
        # Stored (uncompressed) so zipalign can page-align it and the loader can map it in place.
        dst.write(device_lib, "lib/arm64-v8a/libspectrum.so", compress_type=zipfile.ZIP_STORED)

        # Precompiled shaders from `spectrum_tool shaders --target android` (the device has no
        # compiler). MainActivity extracts them to <files>/cache/vulkan, where the engine's
        # shader cache looks.
        blobs = sorted(SHADER_CACHE.glob("*.bin"))
        if not blobs:
            print(f"warning: no shader blobs in {SHADER_CACHE}; run spectrum_tool shaders --target android --out projects/android/shader_cache")
        for blob in blobs:
            dst.write(blob, f"assets/shader_cache/{blob.name}", compress_type=zipfile.ZIP_DEFLATED)

        # Engine assets saved by `spectrum_tool assets` (real .asset files: skin and sky textures, ...).
        # MainActivity extracts them to <files>/assets, where the AssetManager scans.
        for asset in sorted(ENGINE_ASSETS.rglob("*")):
            if asset.is_file():
                dst.write(asset, f"assets/engine_assets/{asset.relative_to(ENGINE_ASSETS).as_posix()}", compress_type=zipfile.ZIP_DEFLATED)

    # 4. align, sign
    aligned = work / "aligned.apk"
    zipalign = [BUILD_TOOLS / "zipalign.exe", "-f"]
    try:
        run([*zipalign, "-P", "16", "4", unaligned, aligned])
    except SystemExit:
        # -P (16 KB page alignment) needs build-tools 35+; fine for 4 KB-page devices only.
        print("warning: zipalign lacks -P, falling back to 4 KB page alignment")
        run([*zipalign, "-p", "4", unaligned, aligned])

    keystore = OUT / "debug.keystore"
    if not keystore.exists():
        run([java_tool("keytool"), "-genkeypair", "-keystore", keystore, "-storepass", "android", "-keypass", "android",
             "-alias", "spectrum", "-keyalg", "RSA", "-keysize", "2048", "-validity", "10000", "-dname", "CN=Spectrum Debug"])
    apk = OUT / args.mode / "spectrum.apk"
    run([BUILD_TOOLS / "apksigner.bat", "sign", "--ks", keystore, "--ks-pass", "pass:android", "--key-pass", "pass:android",
         "--ks-key-alias", "spectrum", "--out", apk, aligned])
    run([BUILD_TOOLS / "apksigner.bat", "verify", apk])
    print(f"{apk}  ({apk.stat().st_size / 1e6:.1f} MB)")

    if args.install:
        adb = SDK / "platform-tools" / "adb.exe"
        print(run([adb, "install", "-r", apk]).stdout.strip())
        print("launch:  adb shell am start -n com.spectrum/.MainActivity   |   logs:  adb logcat -s Spectrum")
    return 0


if __name__ == "__main__":
    sys.exit(main())
