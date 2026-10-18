#!/usr/bin/env python3
"""Install the built APK on the connected device, launch it and stream its log.

    python build/driver.py android run [--mode debug|profile|retail] [--no-install] [--args "--es tests all"]

This is what F5 runs from the Visual Studio SpectrumAndroid solution (as pythonw.exe, debugged, so the
log appears in the Output window's Debug pane). Build first (F7, or build.py then package.py).
Ctrl+C / stopping the debugger ends the log stream; the app keeps running.
"""
import argparse
import ctypes
import device
import os
import subprocess
import sys
import traceback
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]   # repo root: build/platforms/android/<this file>
OUT = ROOT / "projects" / "android"

SDK = next(Path(p) for p in (os.environ.get("ANDROID_HOME"), os.environ.get("ANDROID_SDK_ROOT"),
                             os.path.join(os.environ.get("LOCALAPPDATA", ""), "Android", "Sdk"))
           if p and Path(p).is_dir())
ADB = str(SDK / "platform-tools" / "adb.exe")


def adb(*args, check=True):
    # pythonw has no stdin/console; give the child neither.
    r = subprocess.run([ADB, *args], capture_output=True, text=True, stdin=subprocess.DEVNULL,
                       creationflags=subprocess.CREATE_NO_WINDOW)
    if check and r.returncode != 0:
        sys.exit(f"adb {' '.join(args)} failed:\n{r.stdout}{r.stderr}")
    return (r.stdout + r.stderr).strip()   # adb reports install failures on stderr


def emit(line):
    """To the console, and to the debugger's Output window when one is attached (Visual Studio
    shows OutputDebugString text of the process it debugs; pythonw has no console, hence no print)."""
    if sys.stdout is not None:
        print(line, flush=True)
    if ctypes.windll.kernel32.IsDebuggerPresent():
        ctypes.windll.kernel32.OutputDebugStringW(line + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mode", default="debug", choices=["debug", "profile", "retail"])
    ap.add_argument("--no-install", action="store_true")
    ap.add_argument("--pick", action="store_true", help="always show the device window")
    ap.add_argument("--args", default="", help="extra `am start` arguments, e.g. \"--es tests all\"")
    args = ap.parse_args()

    try:
        device.ensure_device(say=emit, pick=args.pick)   # a window opens when it is not obvious which phone
    except SystemExit as e:
        emit(str(e.code))   # no console under pythonw: the message has to go to the debugger output
        return 1

    apk = OUT / args.mode / "spectrum.apk"
    if not args.no_install:
        if not apk.exists():
            emit(f"{apk} missing: build first")
            return 1
        if not device.install_apk(apk, say=emit):
            return 1

    adb("logcat", "-c")
    adb("shell", "am", "start", "-S", "-n", "com.spectrum/.MainActivity", *args.args.split())
    emit("started; streaming `logcat -s Spectrum` (stop the debugger / Ctrl+C to end)")
    # No console is attached under pythonw, so the child must not try to inherit one.
    log = subprocess.Popen([ADB, "logcat", "-s", "Spectrum"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           stdin=subprocess.DEVNULL, text=True, encoding="utf-8", errors="replace",
                           creationflags=subprocess.CREATE_NO_WINDOW)
    try:
        for line in log.stdout:
            emit(line.rstrip())
    except KeyboardInterrupt:
        pass
    finally:
        log.kill()
    return 0


if __name__ == "__main__":
    # Under pythonw there is no console: an uncaught error or a sys.exit(message) would vanish and the
    # debug session would just end, so everything that goes wrong is sent to the debugger output instead.
    try:
        sys.exit(main())
    except SystemExit as e:
        if isinstance(e.code, str):
            emit(e.code)
            sys.exit(1)
        raise
    except Exception:
        emit(traceback.format_exc())
        sys.exit(1)
