"""Platform-neutral pieces of a Clang C++ modules build (header units, clang-scan-deps, Ninja).

Used by the platform drivers under build/platforms/<name>/ (Android today). A platform supplies the toolchain
(compiler, flags, sysroot), what to compile and its source adaptations; what is here does not depend on it:

  * path helpers for Ninja and change-only file writes (a rewrite bumps the mtime and recompiles dependents),
  * finding Ninja,
  * the dependency scan with a cache (P1689 results keyed on file, command line and headers).

Meant to grow as a second Clang platform (macOS, Linux) is added: header-unit building and the Ninja
emission in the Android driver are the next candidates to move here.
"""
import hashlib
import json
import os
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path


def fwd(p):
    return str(p).replace("\\", "/")


def ninja_path(p):
    return fwd(p).replace("$", "$$").replace(" ", "$ ").replace(":", "$:")


def find_ninja():
    """The Ninja that ships with Visual Studio, else whatever `ninja` is on PATH."""
    vs = Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Microsoft Visual Studio"
    for p in vs.glob("*/*/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe"):
        return p
    return "ninja"


def write_text(path, text, encoding="utf-8"):
    """Write only when the content differs. Everything generated goes through here: a rewrite bumps
    the mtime, and Ninja then recompiles everything that depends on the file, which for a patched
    header or combined header unit is the whole tree."""
    data = text.encode(encoding)
    if path.exists() and path.read_bytes() == data:
        return
    path.write_bytes(data)


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], capture_output=True, text=True, **kw)


def headers_fingerprint(roots, suffixes=(".h", ".hpp", ".inl")):
    """One hash for a set of header trees: path + mtime + size of every header under the roots."""
    h = hashlib.sha1()
    for root in roots:
        for f in sorted(Path(root).rglob("*")):
            if f.suffix in suffixes:
                st = f.stat()
                h.update(f"{f}|{st.st_mtime_ns}|{st.st_size}\n".encode())
    return h.hexdigest()


def scan_p1689(units, cache_file, header_roots, jobs):
    """Runs clang-scan-deps (P1689 format) over `units` = [(path, src, cmd)] in parallel.

    Returns [(path, rule, returncode, stderr)] in the order given; `rule` is the scanner's first rule (what
    the file provides / requires), empty when the scan failed.

    A result depends on the file, the exact command line and, in principle, on the headers it includes, so
    the cache key covers all three. Headers are fingerprinted as one set, not per file: an edited header
    rescans everything (the cost of no cache), while the common edit (a .cpp/.ixx body) rescans only that
    file. Failed scans are not cached: they are rerun, and so reported, every time.
    """
    cache_file = Path(cache_file)
    try:
        cache = json.loads(cache_file.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        cache = {}
    stamp = headers_fingerprint(header_roots)
    fresh, hits = {}, [0]

    def scan(unit):
        path, src, cmd = unit
        st = Path(src).stat()
        key = hashlib.sha1(("\0".join(map(str, cmd)) + f"|{st.st_mtime_ns}|{st.st_size}|{stamp}").encode()).hexdigest()
        hit = cache.get(str(path))
        if hit and hit["key"] == key:
            fresh[str(path)] = hit
            hits[0] += 1
            return path, hit["rule"], hit["rc"], hit["err"]
        r = run(cmd)
        try:
            data = json.loads(r.stdout)
            rule = data["rules"][0] if data["rules"] else {}
        except Exception:
            rule = {}
        if rule:
            fresh[str(path)] = dict(key=key, rule=rule, rc=r.returncode, err=r.stderr)
        return path, rule, r.returncode, r.stderr

    with ThreadPoolExecutor(jobs) as ex:
        results = list(ex.map(scan, units))
    print(f"  {hits[0]} of {len(results)} from cache")
    cache_file.write_text(json.dumps(fresh), encoding="utf-8")
    return results
