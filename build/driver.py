#!/usr/bin/env python3
"""One entry point for the platform build tooling.

    python build/driver.py <platform> <command> [arguments...]

    python build/driver.py android prepare --mode profile    # vcpkg ports + precompiled shaders/textures/assets
    python build/driver.py android build   --mode profile --target app
    python build/driver.py android package --mode profile [--install]
    python build/driver.py android run     --mode profile    # install, launch, stream the log
    python build/driver.py android debug   --mode debug -b draw_frame

A platform is a folder build/platforms/<platform>/ with one script per command (<command>.py); the
arguments after the command go to that script unchanged (see its --help). The platform's own source code
(entry point, stand-ins for missing APIs, packaging files) lives in sources/Platform/<Platform>/.
Windows is built by Sharpmake/MSBuild and has no driver.
"""
import subprocess
import sys
from pathlib import Path

PLATFORMS = Path(__file__).resolve().parent / "platforms"


def main(argv):
    available = sorted(p.name for p in PLATFORMS.iterdir() if p.is_dir() and not p.name.startswith("__"))
    if len(argv) < 2 or argv[0] in ("-h", "--help"):
        print(__doc__)
        print("platforms:", ", ".join(available))
        return 0 if argv and argv[0] in ("-h", "--help") else 2

    platform, command, rest = argv[0], argv[1], argv[2:]
    folder = PLATFORMS / platform
    if not folder.is_dir():
        print(f"unknown platform '{platform}'; available: {', '.join(available)}")
        return 2
    script = folder / f"{command}.py"
    if not script.is_file():
        commands = sorted(p.stem for p in folder.glob("*.py") if p.stem not in ("device", "lldb_android"))
        print(f"{platform} has no command '{command}'; available: {', '.join(commands)}")
        return 2
    return subprocess.run([sys.executable, str(script), *rest]).returncode


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
