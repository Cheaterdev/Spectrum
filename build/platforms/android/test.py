#!/usr/bin/env python3
"""Run the engine's tests on the phone and bring the golden-image results back.

    python build/driver.py android test [--mode debug|profile|retail] [--filter NAME] [--no-install]
                                        [--no-references] [--timeout SECONDS]

Installs the APK (unless --no-install), puts workdir/test_references (the golden PNGs the Windows tests
use) into the app's files dir, starts the app in test mode (`--es tests <filter>`; empty/all = everything
that is linked in), waits for the `TESTS:` summary and prints the failures. Images written by a failing
golden-image test (<name>_actual.png and <name>_diff.png) are pulled to projects/android/test_results/.
Exit code 0 when nothing failed. The same references are used as on Windows; tolerances in the tests
already cover cross-vendor differences (see Test.HAL.Rendering).
"""
import argparse
import io
import os
import re
import shlex
import shutil
import subprocess
import sys
import tarfile
import time
from pathlib import Path

import device

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "projects" / "android"
PACKAGE = "com.spectrum"
ADB = device.ADB


def adb(*args, timeout=120, binary=False):
    r = subprocess.run([ADB, *args], capture_output=True, stdin=subprocess.DEVNULL, timeout=timeout)
    return r.stdout if binary else (r.stdout + r.stderr).decode("utf-8", errors="replace").strip()


def run_as(*cmd, **kw):
    # `adb shell` joins its arguments and the phone's shell parses them again, so quote each one
    return adb("shell", "run-as " + PACKAGE + " " + " ".join(shlex.quote(c) for c in cmd), **kw)


# workdir folders the tests read: the golden PNGs and the sample files (textures, models) some tests load.
PUSHED_DIRS = ["test_references", "resources"]


def push_workdir_dir(name):
    src = ROOT / "workdir" / name
    if not src.is_dir():
        print(f"no {src}: tests that need it will fail or create their own")
        return
    # The app's files dir is its working directory (MainActivity.prepareWorkDir), where the tests look. The app
    # cannot read what `adb push` leaves in /data/local/tmp, so stream a tar into `run-as` instead.
    buf = io.BytesIO()
    with tarfile.open(fileobj=buf, mode="w") as tar:
        tar.add(src, arcname=name)
    expected = sum(1 for f in src.rglob("*") if f.is_file())
    for attempt in (1, 2):   # right after an install the first push has come up short once
        run_as("sh", "-c", f"rm -rf files/{name}")
        r = subprocess.run([ADB, "exec-in", "run-as", PACKAGE, "tar", "-C", "files", "-xf", "-"], input=buf.getvalue(),
                           capture_output=True, timeout=300)
        count = run_as("sh", "-c", f"find files/{name} -type f | wc -l")
        print(f"{name}: {count} of {expected} files in the app's files dir")
        if count == str(expected):
            return
    sys.exit(f"pushing {name} failed: {(r.stdout + r.stderr).decode(errors='replace')}")


def push_references():
    for name in PUSHED_DIRS:
        push_workdir_dir(name)


def pull_results():
    dest = OUT / "test_results"
    shutil.rmtree(dest, ignore_errors=True)
    data = adb("exec-out", "run-as", PACKAGE, "tar", "-C", "files", "-cf", "-", "test_results", binary=True)
    if not data:
        return 0
    try:
        with tarfile.open(fileobj=io.BytesIO(data)) as tar:
            tar.extractall(OUT)
    except tarfile.TarError:
        return 0
    return len(list(dest.glob("*.png"))) if dest.is_dir() else 0


def progress_line(log, elapsed):
    """One status line from the app's log so far: a bar, tests finished of the total, the running test, elapsed time."""
    passed = len(re.findall(r"\[PASS\]", log))
    failed = len(re.findall(r"\[FAIL\]", log))
    skipped = len(re.findall(r"\[SKIP\]", log))
    total_m = re.search(r"Running (\d+) test", log)
    total = int(total_m.group(1)) if total_m else 0
    done = passed + failed + skipped
    started = re.findall(r">> Starting: (\S+)", log)
    current = started[-1] if started else "starting up"
    bar = f"[{'#' * (20 * done // total)}{'.' * (20 - 20 * done // total)}] {done}/{total}" if total else f"{done} done"
    return f"{bar}  {passed} ok, {failed} failed, {skipped} skipped  {int(elapsed)}s  {current}"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mode", default="debug", choices=["debug", "profile", "retail"])
    ap.add_argument("--filter", default="all", help="test name filter, e.g. Core.Serialization (default: all)")
    ap.add_argument("--no-install", action="store_true")
    ap.add_argument("--no-references", action="store_true", help="do not (re)push workdir/test_references")
    ap.add_argument("--timeout", type=int, default=900)
    ap.add_argument("--pick", action="store_true", help="always show the device window")
    args = ap.parse_args()
    sys.stdout.reconfigure(errors="replace")   # failure messages contain characters a cp1252 console cannot print

    device.ensure_device(pick=args.pick)

    apk = OUT / args.mode / "spectrum.apk"
    if not args.no_install:
        if not apk.exists():
            sys.exit(f"{apk} missing: build first (driver.py android build/package --mode {args.mode})")
        if not device.install_apk(apk):
            sys.exit("install failed")

    if not args.no_references:
        push_references()
    run_as("sh", "-c", "rm -rf files/test_results")

    adb("logcat", "-c")
    adb("shell", "am", "start", "-S", "-n", f"{PACKAGE}/.MainActivity", "--es", "tests", args.filter)
    print(f"running tests on the phone (filter '{args.filter}')...", flush=True)

    summary = None
    deadline = time.time() + args.timeout
    log = ""
    started_at = time.time()
    live = sys.stdout.isatty()   # one line rewritten in place; a pipe or log gets a new line per change instead
    last_shown = ""
    while time.time() < deadline:
        time.sleep(2)
        log = adb("logcat", "-d", "-s", "Spectrum")
        m = re.search(r"TESTS: (\d+) total, (\d+) passed, (\d+) failed, (\d+) skipped", log)
        if m:
            summary = tuple(int(x) for x in m.groups())
            break
        shown = progress_line(log, time.time() - started_at)
        if live:
            width = shutil.get_terminal_size().columns - 1
            print(chr(13) + shown[:width].ljust(width), end="", flush=True)
        else:
            changed = progress_line(log, 0)
            if changed != last_shown:
                print(shown, flush=True)
            last_shown = changed
    if live:
        print()
    if not summary:
        print(log[-3000:])
        sys.exit(f"no TESTS: summary within {args.timeout} s (app crashed or still running?); log tail above")

    (OUT / "test_log.txt").write_text(log, encoding="utf-8")
    for line in log.splitlines():
        if re.search(r"\bFAIL|failed|ERROR", line) and "TESTS:" not in line:
            print(line.split("Spectrum: ", 1)[-1])
    images = pull_results()
    total, passed, failed, skipped = summary
    print(f"\n{total} tests: {passed} passed, {failed} failed, {skipped} skipped"
          + (f"; {images} image(s) in {OUT / 'test_results'}" if images else ""))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
