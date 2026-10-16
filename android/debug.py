#!/usr/bin/env python3
"""Attach the NDK's lldb to the running Spectrum app on the device (source-level debugging).

    python android/debug.py [--mode debug|profile|retail] [--launch] [-b FUNC ...] [--batch "cmd; cmd"]

Starts the app if it is not running (--launch restarts it), puts the NDK's lldb-server inside the app's
sandbox (the APK is debuggable, so `run-as` works), forwards it over adb and runs the NDK lldb against
it, attached to the app. Symbols come from the unstripped library the build left in
projects/android/<mode>/lib, so build first. The `debug` mode is compiled with -g (full variables and
types); `profile` has line tables only, so you get stacks and breakpoints but not locals.

Useful lldb commands:  b File.cpp:123 | b Class::method | c | n | s | bt | frame variable [-D N] [name] | thread list | detach
SIGSEGV/SIGBUS are passed to the app without stopping (ART raises them itself); --crashes stops on those from our code.
Exiting lldb detaches and the app keeps running. Breakpoints in code that already ran at startup
need --launch with a breakpoint set before `c`; use -b FUNC to set it right after attaching.
"""
import argparse
import device
import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "projects" / "android"
PACKAGE = "com.spectrum"
SOCKET = f"{PACKAGE}/platform.sock"

SDK = next(Path(p) for p in (os.environ.get("ANDROID_HOME"), os.environ.get("ANDROID_SDK_ROOT"),
                             os.path.join(os.environ.get("LOCALAPPDATA", ""), "Android", "Sdk"))
           if p and Path(p).is_dir())
ADB = str(SDK / "platform-tools" / "adb.exe")
NDK = max((d for d in (SDK / "ndk").iterdir() if d.is_dir()), key=lambda d: [int(x) for x in d.name.split(".") if x.isdigit()])
LLVM = NDK / "toolchains" / "llvm" / "prebuilt" / "windows-x86_64"
LLDB = LLVM / "bin" / "lldb.exe"


def adb(*args, check=True):
    r = subprocess.run([ADB, *args], capture_output=True, text=True, stdin=subprocess.DEVNULL)
    if check and r.returncode != 0:
        sys.exit(f"adb {' '.join(args)} failed:\n{r.stdout}{r.stderr}")
    return r.stdout.strip()


def app_pid():
    return adb("shell", "pidof", PACKAGE, check=False).split()[0:1]


def find_lldb_server():
    found = sorted(LLVM.glob("lib/clang/*/lib/linux/aarch64/lldb-server"))
    if not found:
        sys.exit(f"no aarch64 lldb-server under {LLVM}")
    return found[-1]


def install_server():
    server = find_lldb_server()
    have = adb("shell", "run-as", PACKAGE, "stat", "-c", "%s", "files/lldb-server", check=False)
    if have == str(server.stat().st_size):
        return
    print(f"pushing {server.name} ({server.stat().st_size / 1e6:.0f} MB) into the app sandbox...")
    adb("push", str(server), "/data/local/tmp/lldb-server")
    adb("shell", "run-as", PACKAGE, "cp", "/data/local/tmp/lldb-server", "files/lldb-server")
    adb("shell", "run-as", PACKAGE, "chmod", "755", "files/lldb-server")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mode", default="debug", choices=["debug", "profile", "retail"])
    ap.add_argument("--launch", action="store_true", help="(re)start the app before attaching")
    ap.add_argument("-b", "--break", dest="breaks", action="append", default=[], metavar="LOCATION",
                    help="breakpoint to set right after attaching (function or file:line); repeatable")
    ap.add_argument("--batch", default="", help="lldb commands separated by ';' to run non-interactively, then exit")
    ap.add_argument("--pick", action="store_true", help="always show the device window")
    ap.add_argument("--server", action="store_true",
                    help="only start the app and lldb-server and wait (for VS Code/CodeLLDB to attach)")
    ap.add_argument("--crashes", action="store_true",
                    help="stop on SIGSEGV/SIGBUS raised from our library (a crash in the engine); a stop hook resumes "
                         "the runtime's own ones, but it is not fully reliable, so this is off by default")
    args = ap.parse_args()

    libdir = OUT / args.mode / "lib"
    if not (libdir / "libspectrum.so").exists():
        sys.exit(f"{libdir / 'libspectrum.so'} missing: build first (--mode {args.mode})")
    device.ensure_device(pick=args.pick)   # a window opens when it is not obvious which phone

    if args.launch or not app_pid():
        adb("shell", "am", "start", "-S", "-n", f"{PACKAGE}/.MainActivity")
        for _ in range(30):
            if app_pid():
                break
            time.sleep(0.5)
    pid = app_pid()
    if not pid:
        sys.exit("the app did not start")
    time.sleep(1)   # let it get past the early startup before the ptrace attach
    pid = pid[0]

    install_server()
    adb("shell", "run-as", PACKAGE, "pkill", "-f", "lldb-server", check=False)
    server = subprocess.Popen([ADB, "shell", "run-as", PACKAGE, "./files/lldb-server", "platform", "--server",
                               "--listen", f"unix-abstract:///{SOCKET}"],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL)
    time.sleep(1.5)

    if args.server:
        # For an editor that drives lldb itself (VS Code + CodeLLDB, see .vscode/launch.json): the app is
        # running and the server listens; the editor attaches by name and sets its own breakpoints.
        # The pid changes on every app start and lldb cannot attach by name here, so the editor sources this file.
        attach = OUT / args.mode / "attach.lldb"
        attach.write_text("\n".join([
            f"process attach --pid {pid}",
            f"target symbols add {(libdir / 'libspectrum.so').as_posix()}",
            "process handle SIGPWR SIGXCPU SIGCHLD SIGUSR1 SIGUSR2 SIGSTOP SIGSEGV SIGBUS -s false -p true -n false",
        ]) + "\n")
        print(f"READY: {PACKAGE} is running (pid {pid}), lldb-server listening on @/{SOCKET}. "
              "Stop this task to shut the server down.", flush=True)
        try:
            server.wait()
        except KeyboardInterrupt:
            pass
        finally:
            server.kill()
            adb("shell", "run-as", PACKAGE, "pkill", "-f", "lldb-server", check=False)
        return 0

    # remote-android sets up the adb forwarding itself (an empty serial means the only device) and
    # understands the gdbserver socket the platform server hands back on attach, which remote-linux
    # does not ("invalid host:port specification").
    cmds = ["platform select remote-android",
            f"platform connect unix-abstract-connect:///{SOCKET}",
            # the device paths of our library resolve to the unstripped host copy by name + build-id
            f"settings append target.exec-search-paths {libdir}",
            f"command script import {Path(__file__).with_name('lldb_android.py')}",
            # printing a whole HAL::Device at the default depth (5) crashes the NDK's Windows lldb
            # (access violation in liblldb); a shallow default avoids it, `frame variable -D N x` goes deeper
            "settings set target.max-children-depth 1",
            # ART's JIT-debug-registration hook is an internal lldb breakpoint that editors report as a stop
            "settings set plugin.jit-loader.gdb.enable off",
            f"attach -p {pid}",
            # the APK carries a copy without DWARF (debug mode); the full one stays on the host
            f"target symbols add {libdir / 'libspectrum.so'}",
            # ART raises SIGSEGV/SIGBUS itself (implicit null checks), so by default every signal is passed to
            # the app without stopping; --crashes stops on SIGSEGV/SIGBUS when our library is on the stack
            "process handle SIGPWR SIGXCPU SIGCHLD SIGUSR1 SIGUSR2 SIGSTOP SIGSEGV SIGBUS -s false -p true -n false"]
    if args.crashes:
        cmds += ["process handle SIGSEGV SIGBUS -s true -p true -n false",
                 "target stop-hook add -P lldb_android.IgnoreForeignSignals"]
    cmds += [f"breakpoint set --name {b}" if ":" not in b else f"breakpoint set --file {b.split(':')[0]} --line {b.split(':')[1]}"
             for b in args.breaks]
    batch = [c.strip() for c in args.batch.split(";") if c.strip()]

    # NDK lldb needs its bundled python on PATH (lldb.cmd does the same).
    env = dict(os.environ, PYTHONHOME=str(LLVM / "python3"), PATH=f"{LLVM / 'python3'}{os.pathsep}{os.environ['PATH']}")
    cmd = [str(LLDB)]
    if batch:
        cmd += ["-b"]
    for c in cmds + batch:
        cmd += ["-o", c]
    print(f"attaching lldb to pid {pid} ({args.mode})...")
    try:
        rc = subprocess.run(cmd, env=env).returncode
    finally:
        server.kill()
        adb("shell", "run-as", PACKAGE, "pkill", "-f", "lldb-server", check=False)
    return rc


if __name__ == "__main__":
    sys.exit(main())
