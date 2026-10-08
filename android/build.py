#!/usr/bin/env python3
"""Android (arm64) build driver for Spectrum's C++ modules.

CMake can't build header units (`export import "x.h";`), which every Modules/
wrapper uses, and AGDE has no module support at all. So: build header units
first, scan the sources with clang-scan-deps (P1689), then emit a Ninja file
with exact module/header-unit dependencies and let Ninja run it.

    python android/build.py [--mode debug|retail] [--target core] [-k]

Needs only the Android SDK/NDK (auto-detected, or ANDROID_HOME /
ANDROID_NDK_HOME) and the Ninja that ships with Visual Studio.
"""
import argparse
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "sources"
OUT = ROOT / "projects" / "android"

API = 29
ABI_TRIPLE = "aarch64-linux-android"

# name -> (source dirs relative to sources/, forced-include header, dependencies)
# Grows layer by layer as each one is ported.
TARGETS = {
    "modules": dict(
        dirs=["Modules/stl", "Modules/magic_enum", "Modules/crc32", "Modules/cereal",
              "Modules/crossguid", "Modules/zlib", "Modules/vulkan", "Modules/meshoptimizer", "Modules/skribidi"],
        extra=["android/shims/windows.ixx", "android/shims/ppl.ixx"],   # Android stand-ins
        root="Modules",           # project root: MSVC adds it to the include path
        forced="Modules/Defines.h",
        deps=[],
    ),
    "core": dict(
        dirs=["Core"],
        root="Core",
        forced="Core/Defines.h",
        deps=["modules"],
    ),
    "hal": dict(
        dirs=["HAL"],
        # D3D12/DXGI don't exist here; DXC (the runtime shader compiler) is
        # Windows-only, Android ships precompiled SPIR-V.
        exclude_dirs=["HAL/API/D3D12", "HAL/DXGI", "HAL/DXC"],
        extra=["android/shims/HAL.API.ShaderCompiler.ixx", "android/shims/HAL.ShaderCompiler.android.cpp",
               "android/shims/HAL.NvidiaStubs.ixx"],
        root="HAL",
        forced="HAL/Defines.h",
        deps=["core"],
    ),
    "app": dict(
        dirs=[],
        extra=["android/app/android_entry.cpp"],   # JNI_OnLoad -> Application singleton + logs
        root="Core",
        forced="Core/Defines.h",
        deps=["rendersystem"],
    ),
    "rendersystem": dict(
        dirs=["RenderSystem"],
        # legacy raw4 mesh format: raw4.h does `import Core;` and its .cpp files then include C
        # headers textually, which can't coexist with the combined header unit
        exclude_dirs=["RenderSystem/Mesh/raw4"],
        root="RenderSystem",
        forced="RenderSystem/Defines.h",
        deps=["hal"],
    ),
}

# Files that cannot build on Android yet (Windows-only etc.), relative to sources/.
EXCLUDE = {
    # NVIDIA-only: Streamline / DLSS / NRD
    "HAL/HAL.Streamline.ixx", "HAL/HAL.Streamline.cpp", "HAL/API/Vulkan/HAL.Vulkan.Streamline.ixx",
    "HAL/HAL.DLSS.ixx", "HAL/HAL.DLSS.cpp", "HAL/HAL.DLSSRR.ixx", "HAL/HAL.DLSSRR.cpp",
    "HAL/HAL.NRD.ixx", "HAL/HAL.NRD.cpp",
    # DirectXTex / WIC are Windows-only; Android needs its own image loaders.
    "HAL/HAL.DirectXTexLoad.cpp", "HAL/API/Vulkan/HAL.Vulkan.TextureData.cpp",
    # needs DXC's reflection API
    "HAL/API/Vulkan/HAL.Vulkan.ShaderReflection.cpp",
    # 3D pass bodies. They define PassDefault<Passes::X>::render()/setup(), members of classes
    # declared in module FrameGraph:Passes, from Graphics-module files; MSVC allows that, Clang
    # does not (a member must be defined in its class's module). The declarations still
    # compile, so the pipelines that name these passes build; only the bodies are missing.
    # Revisit by compiling these as `module FrameGraph;` implementation units.
    "RenderSystem/FrameGraph/autogen/pass_defaults.cpp", "RenderSystem/FrameGraph/PassDefaults.cpp",
    "RenderSystem/Effects/FSR/FSR.cpp", "RenderSystem/Effects/Sky.cpp",
    "RenderSystem/Effects/PostProcess/Bloom.cpp", "RenderSystem/Effects/PostProcess/Tonemap.cpp",
    "RenderSystem/Effects/DDGI/DDGIGraph.cpp", "RenderSystem/Effects/VoxelGI/VoxelGIGraph.cpp",
    "RenderSystem/Effects/VoxelGI/RTXCombine.cpp", "RenderSystem/Effects/VoxelGI/NRD_GBufferPack.cpp",
    "RenderSystem/Effects/VoxelGI/NRD_ShadowCombine.cpp", "RenderSystem/Effects/VoxelGI/NRD_IndirectCombine.cpp",
    "RenderSystem/Effects/VoxelGI/NRD_SIGMA_Execute.cpp", "RenderSystem/Effects/VoxelGI/NRD_REBLUR_Execute.cpp",
    "RenderSystem/Effects/VoxelGI/ReflectionRTX.cpp", "RenderSystem/Effects/VoxelGI/ShadowRTX.cpp",
    "RenderSystem/Effects/VoxelGI/IndirectRTX.cpp",
    "RenderSystem/Scene/SceneSystem.cpp", "RenderSystem/Scene/PreSceneSystem.cpp",
    # plain files that include C headers textually after `import Core;` / editor-only rendering
    "RenderSystem/Mesh/MeshletGeneration.cpp", "RenderSystem/Renderer/StencilRenderer.cpp",
    # the DLSS passes call the SDK; the .ixx files only hold portable settings globals
    "RenderSystem/Effects/DLSS/UpscalingDLSS.cpp", "RenderSystem/Effects/DLSS/UpscalingDLSSRR.cpp",
    # generated snapshot code that needs the Graph&-taking functions removed from the header unit
    "RenderSystem/FrameGraph/autogen/context_snapshot.cpp",
    # assimp has no Android port wired up yet
    "RenderSystem/Mesh/raw4/Raw4Loader.cpp", "RenderSystem/Mesh/AssimpLoader.cpp",
    # JSON archives are only used by the Prism host tool; leaving them out also keeps the JSON
    # archive (and its wstring problems) out of every TU that imports the shared header unit
    "Modules/cereal/cereal.json.ixx",
}

# Compiled as plain translation units: vulkan.cpp only provides VMA's implementation
# through global-module-fragment includes. Mixing those textual includes with the
# combined header unit hides bionic's cdefs macros (__BEGIN_DECLS...), and VMA's
# functions have ordinary global linkage either way, so the module line is dropped.
PLAIN_TU = {"Modules/vulkan/vulkan.cpp"}

# Other TUs include <vulkan/vulkan.h> textually in their global module fragment. That
# clashes with the combined header unit (the textual bionic headers are treated as
# already included, but their macros are missing), so the include is dropped and the
# header unit is imported right after the module declaration instead.
VK_GMF_INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*(?:<vulkan/vulkan\.h>|<vma/vk_mem_alloc\.h>|<fstream>|"HAL\.Vulkan\.SpirvEntryPoints\.h"|"(?:autogen/)?(?:resource_ids|pass_ids|context_deps)\.h")[^\n]*$', re.M)
# project headers that ride along in the combined header unit (after the Vulkan headers)
COMBINED_EXTRA_HEADERS = {
    "hal": ["HAL/API/Vulkan/HAL.Vulkan.SpirvEntryPoints.h"],
    # generated by Prism, included in global module fragments (context_deps.h pulls in <bitset>)
    "rendersystem": ["RenderSystem/FrameGraph/autogen/resource_ids.h",
                     "RenderSystem/FrameGraph/autogen/pass_ids.h",
                     "RenderSystem/FrameGraph/autogen/context_deps.h"],
}
MODULE_DECL_RE = re.compile(r"^(\s*(?:export\s+)?module\s+[A-Za-z_][\w.]*(?::[A-Za-z_][\w.]*)?\s*;)", re.M)
PLAIN_TU_RE = re.compile(r"^\s*module\s*(?:vulkan)?\s*;[^\n]*$", re.M)

# `import <windows/windows.h>;` appears in GUI/camera code next to `import windows;`.
# Whatever Win32 surface they need comes from android/shims/windows.ixx.
WINDOWS_HEADER_UNITS = {"windows/windows.h"}

# Plain .cpp files that `import GUI;` and then define members of GUI's classes. MSVC lets a
# non-module file do that; in Clang the class is attached to the module, so the file has to
# be (a primary implementation unit of) that module.
IMPL_OF_MODULE = {"RenderSystem/GUI/Renderer/NinePatch.cpp": "GUI",
                  "RenderSystem/Scene/SceneObject.cpp": "Graphics"}

# Partitions removed above: umbrella modules (HAL.ixx) re-export them, so drop those imports.
DROP_MODULE_IMPORTS = {":Streamline", ":DLSS", ":DLSSRR", ":NRD"}
# ...replaced by one import of android/shims/HAL.NvidiaStubs.ixx (same `nvidia::` names, all "unavailable")
NVIDIA_STUB_IMPORT = {":Streamline": "export import :NvidiaStubs;"}
DROP_IMPORT_RE = re.compile(r'^(\s*)(?:export\s+)?import\s+(:[A-Za-z_][\w.]*)\s*;', re.M)


def find_sdk():
    for p in (os.environ.get("ANDROID_HOME"), os.environ.get("ANDROID_SDK_ROOT"),
              os.path.join(os.environ.get("LOCALAPPDATA", ""), "Android", "Sdk")):
        if p and Path(p).is_dir():
            return Path(p)
    sys.exit("Android SDK not found; set ANDROID_HOME")


def find_ndk(sdk):
    p = os.environ.get("ANDROID_NDK_HOME")
    if p and Path(p).is_dir():
        return Path(p)
    root = sdk / "ndk"
    cands = sorted((d for d in root.iterdir() if d.is_dir()), key=lambda d: [int(x) for x in re.findall(r"\d+", d.name)]) if root.is_dir() else []
    if not cands:
        sys.exit(f"no NDK under {root}")
    return cands[-1]


def find_ninja():
    vs = Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Microsoft Visual Studio"
    for p in vs.glob("*/*/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe"):
        return p
    return "ninja"


SDK = find_sdk()
NDK = find_ndk(SDK)
BIN = NDK / "toolchains" / "llvm" / "prebuilt" / "windows-x86_64" / "bin"
CLANGXX = BIN / "clang++.exe"
SCAN = BIN / "clang-scan-deps.exe"
AR = BIN / "llvm-ar.exe"
NINJA = find_ninja()

IMPORT_RE = re.compile(r'(?:^|;)[ \t]*(?:export\s+)?import\s+(<[^>]+>|"[^"]+")\s*;', re.M)


def fwd(p):
    return str(p).replace("\\", "/")


def ninja_path(p):
    return fwd(p).replace("$", "$$").replace(" ", "$ ").replace(":", "$:")


INCLUDE_ROOTS = [SRC]


def base_flags(mode):
    f = [f"--target={ABI_TRIPLE}{API}", "-std=c++2b", "-fPIC", "-fexceptions", "-frtti",
         "-Wno-experimental-header-units", "-Wno-unknown-attributes",
         "-fdeclspec",   # CACHE_ALIGN() etc. use __declspec(align(...))
         "-Wno-unused-command-line-argument",   # -I / -include are ignored when compiling a .pcm to an object
         *[f"-I{fwd(d)}" for d in INCLUDE_ROOTS]]
    f += ["-DSPECTRUM_PLATFORM_ANDROID", "-DSPECTRUM_ENABLE_EXCEPTIONS", "-DHAL_BACKEND_VULKAN", "-DLEAK_TEST_ENABLE"]
    if mode == "debug":
        f += ["-O0", "-gline-tables-only", "-DDEV", "-DPROFILING"]
    else:
        f += ["-O2", "-DRETAIL", "-DNDEBUG"]
    return f


def rel(p):
    """Path relative to sources/, or to the repo root for files outside it (android/shims)."""
    try:
        return fwd(p.relative_to(SRC))
    except ValueError:
        return fwd(p.relative_to(ROOT))


def gather(target_names):
    """target -> list of source files, following deps; each file tagged with its target."""
    order, seen = [], set()

    def visit(n):
        if n in seen:
            return
        seen.add(n)
        for d in TARGETS[n]["deps"]:
            visit(d)
        order.append(n)
    for n in target_names:
        visit(n)
    files = {}
    for n in order:
        lst = []
        for d in TARGETS[n]["dirs"]:
            base = SRC / d
            for p in sorted(list(base.rglob("*.ixx")) + list(base.rglob("*.cpp"))):
                if rel(p) in EXCLUDE or any(rel(p).startswith(x + "/") for x in TARGETS[n].get("exclude_dirs", [])):
                    continue
                lst.append(p)
        lst += [ROOT / e for e in TARGETS[n].get("extra", [])]
        files[n] = lst
    return order, files


# Vulkan's surface platform: the sources are written for Win32; Android has its own
# window-system extension. Mapped in the compiled copies so the sources stay as they are.
PLATFORM_RULES = [
    (re.compile(r"#\s*define\s+VK_USE_PLATFORM_WIN32_KHR\b"), "#define VK_USE_PLATFORM_ANDROID_KHR"),
    (re.compile(r"#\s*include\s*<vulkan/vulkan_win32\.h>"), "#include <vulkan/vulkan_android.h>"),
    (re.compile(r"\bVK_KHR_WIN32_SURFACE_EXTENSION_NAME\b"), "VK_KHR_ANDROID_SURFACE_EXTENSION_NAME"),
]


def apply_platform_rules(text):
    for rx, repl in PLATFORM_RULES:
        text = rx.sub(repl, text)
    return text


HEADER_COPIES = OUT / "hdr"   # patched copies of headers that are loaded as header units
EXPORT_PARTS = {}              # patched header path -> `export ...` text moved out of it
EXPORT_LINE_RE = re.compile(r"^\s*export\b", re.M)


def split_exports(text):
    """MSVC lets a header unit contain `export` declarations; Clang requires a module
    purview for them. Split the header at its first `export` (plus the #undef lines
    right before it): the include part stays the header unit, the rest is returned
    to be appended to the wrapper module that imports it."""
    m = EXPORT_LINE_RE.search(text)
    if not m:
        return text, ""
    lines = text[:m.start()].split("\n")
    # lines[-1] is the (empty) start of the export line itself
    cut = len(lines) - 1
    while cut > 0 and (lines[cut - 1].lstrip().startswith(("#undef", "//")) or not lines[cut - 1].strip()):
        cut -= 1
    return "\n".join(lines[:cut]) + "\n", "\n".join(lines[cut:]) + text[m.start():]


def maybe_patch_header(c):
    """Header units are read by path, so a header needing platform rules is copied."""
    text = c.read_text(encoding="utf-8-sig", errors="ignore")
    head, exports = split_exports(text)
    patched = apply_platform_rules(head)
    if patched == text:
        return c
    out = HEADER_COPIES / c.name
    HEADER_COPIES.mkdir(parents=True, exist_ok=True)
    if not out.exists() or out.read_text(encoding="utf-8-sig", errors="ignore") != patched:
        out.write_text(patched, encoding="utf-8")
    if exports:
        EXPORT_PARTS[fwd(out)] = exports
    return out


def resolve_header_unit(spec, importer):
    """Header-unit spec like '"x.h"' or '<x>' -> (key, kind, path|name).
    `importer` is a path, or several (the compiled copy and the original source: relative
    quoted names such as "../defines.h" must resolve against the original's directory)."""
    name = spec[1:-1]
    importers = importer if isinstance(importer, (list, tuple)) else [importer]
    dirs = ([i.parent for i in importers] if spec[0] == '"' else []) + INCLUDE_ROOTS
    for d in dirs:
        c = d / name
        if c.is_file():
            c = maybe_patch_header(c.resolve())
            return fwd(c), "user", c
    return "<" + name + ">", "system", name


PART_IMPL_RE = re.compile(r'^(\s*)module\s+([A-Za-z_][\w.]*):([A-Za-z_][\w.]*)\s*;', re.M)
PRIMARY_IMPL_RE = re.compile(r'^(\s*)module\s+([A-Za-z_][\w.]*)\s*;', re.M)


COMBINED_HEADER = "android_all.h"
WRAP_IMPORT_RE = re.compile(r'^(\s*)(export\s+)?import\s+(<[^>]+>|"[^"]+")\s*;', re.M)


INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


def forced_header_keys(order):
    """Everything the forced-include Defines.h chain pulls in textually."""
    keys, todo = set(), [SRC / TARGETS[t]["forced"] for t in order]
    while todo:
        f = todo.pop()
        if not f.is_file() or fwd(f.resolve()) in keys:
            continue
        keys.add(fwd(f.resolve()))
        for m in INCLUDE_RE.finditer(f.read_text(encoding="utf-8-sig", errors="ignore")):
            for d in [f.parent, *INCLUDE_ROOTS]:
                if (d / m.group(1)).is_file():
                    todo.append(d / m.group(1))
                    break
    return keys


MODULE_IMPORTS = {}   # module name -> non-partition modules imported by any of its interfaces
IFACE_RE = re.compile(r'^\s*export\s+module\s+([A-Za-z_][\w.]*(?::[A-Za-z_][\w.]*)?)\s*;', re.M)
MODIMP_RE = re.compile(r'^\s*(?:export\s+)?import\s+([A-Za-z_:][\w.:]*)\s*;', re.M)


def patched_source(p, gen_dir, wrapper=False, drop=frozenset(), ifaces={}):
    """Feed Clang standard-conforming copies of sources MSVC accepts as written
    (the real sources stay untouched, diagnostics keep their line numbers):

    * `module M:P;` in a .cpp: MSVC treats it as the implementation of the interface
      partition M:P and lets it see that interface. In Clang it is an unrelated
      partition implementation unit that imports nothing, so it becomes
          module M:P__impl; import :P;
    * Modules/ wrappers (`export import "x.h";`): several header units that each
      include the same libc++ headers aren't merged by Clang ("reference to 'string'
      is ambiguous"), so every wrapper imports one combined header unit instead.
    """
    text = p.read_text(encoding="utf-8-sig", errors="ignore")
    patched = text
    if rel(p) in IMPL_OF_MODULE and not MODULE_DECL_LINE_RE.search(patched):
        patched = f"module {IMPL_OF_MODULE[rel(p)]};\n" + patched
    m = PART_IMPL_RE.search(patched) if p.suffix == ".cpp" else None
    prim = None if m or p.suffix != ".cpp" else PRIMARY_IMPL_RE.search(patched)
    if prim:
        # `module HAL;` already imports HAL; Clang rejects an explicit `import HAL;` in it.
        patched = re.sub(rf"^[ \t]*(?:export\s+)?import\s+{re.escape(prim.group(2))}\s*;[^\n]*$",
                         "// (implicit in a primary module implementation)", patched, flags=re.M)
        if "import stl.core;" not in patched:
            # std names reach these files through MSVC's looser visibility (generated
            # autogen.cpp uses std::optional with no std import at all).
            prim = PRIMARY_IMPL_RE.search(patched)
            extra_imports = [f" import {n};" for n in sorted(MODULE_IMPORTS.get(prim.group(2), ()))
                             if f"import {n};" not in patched]
            patched = patched[:prim.end()] + "".join(extra_imports) + patched[prim.end():]
            prim = PRIMARY_IMPL_RE.search(patched)
            patched = patched[:prim.end()] + " import stl.core;" + patched[prim.end():]
    if m:
        mod, part = m.group(2), m.group(3)
        # MSVC also gives the implementation the interface's own imports; replay them.
        inherited = []
        ip = ifaces.get(f"{mod}:{part}")
        if ip:
            for im in MODIMP_RE.finditer(ip.read_text(encoding="utf-8-sig", errors="ignore")):
                if im.group(1) not in (f":{part}",) and f"import {im.group(1)};" not in inherited:
                    inherited.append(f"import {im.group(1)};")
        # MSVC also makes everything the module's other partitions import visible here.
        for n in sorted(MODULE_IMPORTS.get(mod, ())):
            if f"import {n};" not in inherited:
                inherited.append(f"import {n};")
        self_import = re.compile(rf"^[ \t]*(?:export\s+)?import\s+{re.escape(mod)}\s*;[^\n]*$", re.M)
        if self_import.search(patched):
            # MSVC lets a partition implementation `import HAL;` (its own module); Clang
            # forbids it. As a primary-module implementation unit it sees all of HAL's
            # exports implicitly, and importing the partition adds its non-exported part.
            patched = self_import.sub("// (implicit: this is compiled as a primary module implementation)", patched)
            m = PART_IMPL_RE.search(patched)
            # ...but partitions the primary interface doesn't re-export (HAL:API.*) stay
            # invisible, while MSVC's `import HAL;` there sees them: import them too.
            api = [f"import :{k.split(':', 1)[1]};" for k in sorted(ifaces)
                   if k.startswith(mod + ":API.") and k != f"{mod}:{part}"]
            # `HAL:API.CommandAllocator` implements members of HAL::CommandAllocator, which
            # lives in the same-named non-API partition; out-of-line definitions need it imported.
            twin = part.split("API.", 1)[1] if part.startswith("API.") else None
            if twin and f"{mod}:{twin}" in ifaces:
                api.insert(0, f"import :{twin};")
            new = f"{m.group(1)}module {mod}; import :{part}; {' '.join(inherited + api)}"
        else:
            # Unique per source file: HAL.QueryHeap.cpp and HAL.Vulkan.QueryHeap.cpp both implement
            # :QueryHeap, and two units named :QueryHeap__impl export the same module-initializer symbol.
            unique = re.sub(r"[^A-Za-z0-9_]", "_", rel(p).rsplit(".", 1)[0])
            new = f"{m.group(1)}module {mod}:{part}__impl_{unique}; import :{part}; {' '.join(inherited)}"
        patched = patched[:m.start()] + new + patched[m.end():]
    if drop:
        # A header that is both forced-included and loaded as a header unit loses its
        # macros in Clang (the #include is translated into an import). These are
        # macro-only headers the forced include already delivers, so the import is redundant.
        def drop_import(mm):
            if mm.group(3)[1:-1] in WINDOWS_HEADER_UNITS:
                return f"{mm.group(1)}import windows;"   # no Windows header unit here; use the shim module
            key, _, _ = resolve_header_unit(mm.group(3), p)
            return f"{mm.group(1)}//" if key in drop else mm.group(0)
        patched = WRAP_IMPORT_RE.sub(drop_import, patched)
    if wrapper:
        moved = []

        def to_combined(mm):
            _, kind, hdr = resolve_header_unit(mm.group(3), p)
            if kind == "user" and fwd(hdr) in EXPORT_PARTS:
                moved.append(EXPORT_PARTS[fwd(hdr)])
            return f'{mm.group(1)}{mm.group(2) or ""}import "{COMBINED_HEADER}";'
        patched = WRAP_IMPORT_RE.sub(to_combined, patched)
        if moved:
            patched += "\n// ---- `export` declarations moved out of the header unit (see split_exports)\n" + "\n".join(moved)
    if rel(p) in PLAIN_TU:
        patched = PLAIN_TU_RE.sub("", patched)
    elif not wrapper and VK_GMF_INCLUDE_RE.search(patched):
        patched = VK_GMF_INCLUDE_RE.sub("// <vulkan/vulkan.h> comes from the combined header unit", patched)
        patched = MODULE_DECL_RE.sub(lambda mm: mm.group(1) + f' import "{COMBINED_HEADER}";', patched, count=1)
    patched = DROP_IMPORT_RE.sub(
        lambda mm: (f"{mm.group(1)}{NVIDIA_STUB_IMPORT[mm.group(2)]}" if mm.group(2) in NVIDIA_STUB_IMPORT
                    else f"{mm.group(1)}//" if mm.group(2) in DROP_MODULE_IMPORTS else mm.group(0)), patched)
    if rel(p) in PURVIEW_EXTERN_C_INCLUDES:
        mirror_pass_headers(p.parent, gen_dir)
        patched = wrap_purview_includes(patched, PURVIEW_EXTERN_C_INCLUDES[rel(p)],
                                        prelude=[gen_dir / "PassNodeBase.h"])
        # PassDefault<T> is textually included by ~25 files; each module-purview inclusion would
        # be a separate module-attached definition under Clang ("redefinition of PassDefault").
        # It is defined once, via the pass headers below (they include pass_defaults.h themselves);
        # every other file sees it through FrameGraph.
    elif PASS_DEFAULTS_INCLUDE_RE.search(patched):
        patched = PASS_DEFAULTS_INCLUDE_RE.sub("// pass_defaults.h is defined once, in FrameGraph:Passes", patched)
    if rel(p) in PASS_MEMBER_SPLIT:
        patched = split_pass_members(patched, PASS_MEMBER_SPLIT[rel(p)], gen_dir)
    patched = apply_platform_rules(patched)
    patched = hoist_imports(patched)
    if patched == text:
        return p
    out = gen_dir / rel(p).replace("/", "__")
    gen_dir.mkdir(parents=True, exist_ok=True)
    if not out.exists() or out.read_text(encoding="utf-8-sig", errors="ignore") != patched:
        out.write_text(patched, encoding="utf-8")
    return out


IMPORT_LINE_RE = re.compile(r"(?:export\s+)?import\s+[^;]+;\s*(?://.*)?$")
MODULE_DECL_LINE_RE = re.compile(r"\s*(?:export\s+)?module\s+[\w.:]+\s*;")


def hoist_imports(text):
    """Clang requires every import to come directly after the module declaration; MSVC
    accepts them later (e.g. after macro-generated registration code). Move late imports up."""
    lines = text.split("\n")
    decl = next((i for i, l in enumerate(lines) if MODULE_DECL_LINE_RE.match(l)), None)
    if decl is None:
        return text
    keep, late, seen_code = lines[: decl + 1], [], False
    for l in lines[decl + 1:]:
        s = l.strip()
        is_import = bool(IMPORT_LINE_RE.match(s))
        if is_import and seen_code:
            late.append(l)
            continue
        if s and not is_import and not s.startswith(("//", "/*", "*", "#")):
            seen_code = True
        keep.append(l)
    if not late:
        return text
    return "\n".join(keep[: decl + 1] + late + keep[decl + 1:])


# Files whose `#include "pass/..."` block sits in the module purview. Those declarations are
# attached to the module (module linkage), so `export using ::Passes::X;` below them is
# rejected by Clang. Wrapping the include block in `export { }` exports the declarations
# directly (they stay attached to the module, which the module's own .cpp files need).
PASS_DEFAULTS_INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*"[^"]*pass_defaults\.h"[^\n]*$', re.M)
PURVIEW_EXTERN_C_INCLUDES = {"RenderSystem/FrameGraph/autogen/passes.ixx": re.compile(r'^#include "pass/', re.M)}


def wrap_purview_includes(text, include_start_re, prelude=()):
    """`prelude` headers are included first, still at import level (they contain `import`
    statements, which are not allowed inside the export block)."""
    hits = list(include_start_re.finditer(text))
    if not hits:
        return text
    first = hits[0].start()
    last_line_end = text.find("\n", hits[-1].start())
    last_line_end = len(text) if last_line_end < 0 else last_line_end
    pre = "".join(f'#include "{fwd(h)}"\n' for h in prelude)
    return (text[:first] + pre + 'export {\n' + text[first:last_line_end] + "\n}" + text[last_line_end:])


def mirror_pass_headers(autogen, gen_dir):
    """Copy autogen/pass/** and PassNodeBase.h next to the patched passes.ixx in gen/, with the
    extern "C++" wrapper removed from pass_defaults.h. Quoted includes resolve relative to the
    including file first, so the pass headers' "../../pass_defaults.h" now finds the copy."""
    def sync(src, dst, text=None):
        data = text if text is not None else src.read_bytes()
        if isinstance(data, str):
            data = data.encode("utf-8")
        dst.parent.mkdir(parents=True, exist_ok=True)
        if not dst.exists() or dst.read_bytes() != data:
            dst.write_bytes(data)
    for h in (autogen / "pass").rglob("*.h"):
        sync(h, gen_dir / "pass" / h.relative_to(autogen / "pass"))
    sync(autogen / "PassNodeBase.h", gen_dir / "PassNodeBase.h")
    sync(None, gen_dir / "pass_defaults.h", patched_pass_defaults(autogen / "pass_defaults.h").read_text(encoding="utf-8"))


def patched_pass_defaults(src):
    """The generated header wraps everything in extern "C++" { } so it lands in the global
    module on MSVC. Under Clang that would make PassDefault<T>::render() a global-module
    declaration while the module units that define it are module-attached ("declaration of
    'render' in module ... follows declaration in the global module"), so the wrapper goes."""
    text = src.read_text(encoding="utf-8-sig", errors="ignore")
    text, n = re.subn(r'^extern "C\+\+"\s*\{', "", text, count=1, flags=re.M)
    if n:
        text = text.rstrip()
        assert text.endswith("}"), "pass_defaults.h no longer ends with the extern block's brace"
        text = text[:-1] + "\n"
    out = HEADER_COPIES / "autogen" / src.name
    out.parent.mkdir(parents=True, exist_ok=True)
    if not out.exists() or out.read_text(encoding="utf-8", errors="ignore") != text:
        out.write_text(text, encoding="utf-8")
    return out


# Definitions of PassDefault<Passes::X>::render()/setup() must be in the same module as the
# class (FrameGraph:Passes) under Clang, but the engine writes the UI ones inside GUI/Base.cpp
# (module GUI). They are cut out of the compiled copy and emitted as a `module FrameGraph;`
# implementation unit, which may import GUI because it is not an interface unit.
PASS_MEMBER_SPLIT = {
    "RenderSystem/GUI/Base.cpp": dict(
        move=re.compile(r"^void\s+PassDefault<Passes::UI::\w+>::render\(", re.M),
        copy=re.compile(r"^static\s+uint32_t\s+ui_per_thread\([^)]*\)\s*\{", re.M),   # the definition, not the forward declaration
        out="GUI__UiPassMembers.cpp",
        prelude="using GUI::Renderer;   // the original sits in module GUI, where this is found unqualified"),
}


def _function_span(text, start):
    """[start, end) of the function whose signature begins at `start` (brace matching)."""
    i = text.index("{", start)
    depth = 0
    for j in range(i, len(text)):
        if text[j] == "{":
            depth += 1
        elif text[j] == "}":
            depth -= 1
            if depth == 0:
                return start, j + 1
    raise ValueError("unbalanced braces")


def split_pass_members(text, rule, gen_dir):
    moved, copied = [], []
    while True:
        m = rule["move"].search(text)
        if not m:
            break
        a, b = _function_span(text, m.start())
        moved.append(text[a:b])
        text = text[:a] + "// (moved to " + rule["out"] + ": must be defined in module FrameGraph)\n" + text[b:]
    for m in rule["copy"].finditer(text):
        a, b = _function_span(text, m.start())
        copied.append(text[a:b])
    # imports can share the module-declaration line, so scan the header region by regex
    first_code = re.search(r"^(?:namespace|class|struct|template|static|void|const|using)\b", text, re.M)
    header = text[: first_code.start()] if first_code else text
    imports = [f"import {n};" for n in re.findall(r"\bimport\s+([A-Za-z_][\w.]*)\s*;", header) if n != "FrameGraph"]
    out = gen_dir / rule["out"]
    body = ("module FrameGraph;\nimport GUI;\n" + "\n".join(dict.fromkeys(imports)) + "\n\n"
            + rule.get("prelude", "") + "\n\n"
            + "\n\n".join(copied + moved) + "\n")
    if not out.exists() or out.read_text(encoding="utf-8", errors="ignore") != body:
        out.write_text(body, encoding="utf-8")
    return text


def patched_context_deps(src):
    """Prism's generated context_deps.h is written for a global module fragment. In the
    combined header unit it must not (a) include <bitset> textually (the unit lists it
    first, once), nor (b) forward-declare FrameGraph::Graph, which the FrameGraph module
    later defines with module attachment (Clang: "follows declaration in the global
    module"). The two functions taking Graph& are only used by autogen/context_snapshot.cpp,
    which the Android build doesn't compile, so their declarations go too."""
    text = src.read_text(encoding="utf-8-sig", errors="ignore")
    text = text.replace("#include <bitset>", "")
    text = text.replace('#include "pass_ids.h"', f'#include "{fwd(src.parent / "pass_ids.h")}"')
    for pattern in (r"^class Graph;[ 	]*$",
                    r"^void capture_context_snapshot\(Graph& graph, ContextSnapshot& out\);[ 	]*$",
                    r"^void update_context_dirty_mask\(Graph& graph\);[ 	]*$"):
        text = re.sub(pattern, "", text, flags=re.M)
    out = HEADER_COPIES / "autogen" / src.name
    out.parent.mkdir(parents=True, exist_ok=True)
    if not out.exists() or out.read_text(encoding="utf-8", errors="ignore") != text:
        out.write_text(text, encoding="utf-8")
    return out


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], capture_output=True, text=True, **kw)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mode", default="debug", choices=["debug", "retail"])
    ap.add_argument("--target", default="core", choices=list(TARGETS))
    ap.add_argument("-k", action="store_true", help="keep going after errors")
    ap.add_argument("--no-build", action="store_true", help="only generate build.ninja")
    ap.add_argument("-j", type=int, default=os.cpu_count())
    args = ap.parse_args()

    out = OUT / args.mode
    out.mkdir(parents=True, exist_ok=True)
    order, files = gather([args.target])
    INCLUDE_ROOTS.extend(SRC / TARGETS[t]["root"] for t in order)
    vcpkg_inc = OUT / "vcpkg_installed" / "arm64-android" / "include"   # see android/vcpkg.json
    if vcpkg_inc.is_dir():
        INCLUDE_ROOTS.append(vcpkg_inc)
    # The Windows vcpkg layout is <vma/vk_mem_alloc.h>; the Android port installs it at the top level.
    vma_fwd = out / "gen" / "include" / "vma" / "vk_mem_alloc.h"
    vma_fwd.parent.mkdir(parents=True, exist_ok=True)
    vma_fwd.write_text("#include <vk_mem_alloc.h>\n")
    INCLUDE_ROOTS.append(out / "gen" / "include")
    flags = base_flags(args.mode)
    allfiles = [(t, p) for t in order for p in files[t]]
    print(f"NDK {NDK.name}, {len(allfiles)} files in {order}")

    # ---- header units -------------------------------------------------------
    gen = out / "gen"
    gen.mkdir(exist_ok=True)
    # one combined header unit for every Modules/ wrapper (see patched_source)
    specs, seen_specs = [], set()
    for t, p in allfiles:
        if t != "modules":
            continue
        for m in WRAP_IMPORT_RE.finditer(p.read_text(encoding="utf-8-sig", errors="ignore")):
            key, kind, src = resolve_header_unit(m.group(3), p)
            if key not in seen_specs:
                seen_specs.add(key)
                specs.append(f'#include "{fwd(src)}"' if kind == "user" else f"#include <{src}>")
    if "hal" in order:
        specs.insert(0, "#include <fstream>")   # HAL TUs include it in their global module fragment
    for t in order:
        for h in COMBINED_EXTRA_HEADERS.get(t, []):
            path = SRC / h
            if path.name == "context_deps.h":
                specs.append("#include <bitset>")
                path = patched_context_deps(path)
            specs.append(f'#include "{fwd(path)}"')
    combined_text = "#pragma once\n" + "\n".join(specs) + "\n"
    combined = gen / COMBINED_HEADER
    if not combined.exists() or combined.read_text() != combined_text:
        combined.write_text(combined_text)

    forced_keys = forced_header_keys(order)
    ifaces = {}
    for _, p in allfiles:
        if p.suffix == ".ixx":
            m = IFACE_RE.search(p.read_text(encoding="utf-8-sig", errors="ignore"))
            if m:
                ifaces[m.group(1)] = p
    for name, ip in ifaces.items():
        mod = name.split(":")[0]
        for im in MODIMP_RE.finditer(ip.read_text(encoding="utf-8-sig", errors="ignore")):
            n = im.group(1)
            if not n.startswith(":") and n != mod:
                MODULE_IMPORTS.setdefault(mod, set()).add(n)
    src_of = {p: patched_source(p, gen, wrapper=(t == "modules"), drop=forced_keys, ifaces=ifaces)
              for t, p in allfiles}

    for rule in PASS_MEMBER_SPLIT.values():
        generated = gen / rule["out"]
        if generated.exists() and not any(q == generated for _, q in allfiles):
            allfiles.append(("rendersystem", generated))
            src_of[generated] = generated

    hu = {}  # key -> dict(kind, src, pcm)
    imports = {}  # file -> [hu keys]
    for _, p in allfiles:
        text = src_of[p].read_text(encoding="utf-8-sig", errors="ignore")
        keys = []
        for m in IMPORT_RE.finditer(text):
            key, kind, src = resolve_header_unit(m.group(1), [src_of[p], p])
            if key not in hu:
                safe = re.sub(r"[^A-Za-z0-9_.]", "_", key)[-100:]
                hu[key] = dict(kind=kind, src=src, pcm=out / "hu" / (safe + ".pcm"))
            keys.append(key)
        imports[p] = keys

    def hu_cmd(h):
        c = [CLANGXX, *flags, "--precompile", "-MD", "-MF", f"{fwd(h['pcm'])}.d", "-o", h["pcm"]]
        if h["kind"] == "user":
            c += ["-fmodule-header=user", "-xc++-user-header", h["src"]]
        else:
            c += ["-fmodule-header=system", "-xc++-system-header", h["src"]]
        return c

    # Phase 1: header units must exist before the scanner can resolve imports of them.
    nin = out / "headerunits.ninja"
    with open(nin, "w") as f:
        f.write("rule hu\n  command = $cmd\n  description = HU $out\n  deps = gcc\n  depfile = $out.d\n\n")
        for h in hu.values():
            cmd = " ".join(f'"{c}"' if " " in str(c) else str(c) for c in hu_cmd(h))
            f.write(f"build {ninja_path(fwd(h['pcm']))}: hu\n  cmd = {cmd}\n")
    (out / "hu").mkdir(exist_ok=True)
    r = subprocess.run([str(NINJA), "-C", str(out), "-f", "headerunits.ninja", "-k", "0"])
    ok_hu = {k for k, h in hu.items() if h["pcm"].exists()}
    bad = [k for k in hu if k not in ok_hu]
    if bad:
        print(f"{len(bad)} header unit(s) failed to build: {bad}")

    # ---- scan ---------------------------------------------------------------
    def hu_flags(keys):
        return [f"-fmodule-file={fwd(hu[k]['pcm'])}" for k in sorted(set(keys)) if k in ok_hu]

    def forced_for(t):
        return [f"-include{fwd(SRC / TARGETS[t]['forced'])}"]

    tu = {}
    for t, p in allfiles:
        objname = rel(p).replace("/", "__") + ".o"
        src = src_of[p]
        # a patched copy lives elsewhere, so its relative #includes need the original dir
        near = [f"-I{fwd(p.parent)}"] if src != p else []
        tu[p] = dict(target=t, obj=out / "obj" / objname, is_ixx=p.suffix == ".ixx", src=src, near=near)

    def scan(p):
        t = tu[p]["target"]
        cmd = [SCAN, "-format=p1689", "--", CLANGXX, *flags, *tu[p]["near"], *forced_for(t), *hu_flags(imports[p]),
               *(["-x", "c++-module"] if p.suffix == ".ixx" else []), "-c", tu[p]["src"], "-o", tu[p]["obj"]]
        r = run(cmd)
        try:
            data = json.loads(r.stdout)
            rule = data["rules"][0] if data["rules"] else {}
        except Exception:
            rule = {}
        return p, rule, r.returncode, r.stderr

    print("scanning...")
    with ThreadPoolExecutor(args.j) as ex:
        results = list(ex.map(scan, [p for _, p in allfiles]))

    provides, requires, scan_fail = {}, {}, []
    for p, rule, rc, err in results:
        if not rule:
            scan_fail.append((p, err.strip().splitlines()[:3]))
            requires[p] = []
            continue
        requires[p] = [r["logical-name"] for r in rule.get("requires", [])]
        for pr in rule.get("provides", []):
            if pr.get("is-interface"):
                provides[pr["logical-name"]] = p
            elif ":" not in pr["logical-name"] and pr["logical-name"] not in requires[p]:
                # `module Core;` implicitly imports the primary interface; P1689 omits that edge.
                # (Partition implementation units are rewritten to import explicitly, see patched_source.)
                requires[p].append(pr["logical-name"])

    missing = {}
    for p, reqs in requires.items():
        for n in reqs:
            if n not in provides:
                missing.setdefault(n, []).append(p)
    if scan_fail:
        print(f"\nscan failed for {len(scan_fail)} file(s):")
        for p, e in scan_fail:
            print("  ", rel(p), "|", " / ".join(e))
    if missing:
        print("\nimports with no provider (these files are skipped):")
        for n, ps in sorted(missing.items()):
            print(f"  {n}: {len(ps)} file(s), e.g. {rel(ps[0])}")

    # a file is buildable if its whole required-module closure has providers
    memo = {}

    def closure(p, stack=()):
        if p in memo:
            return memo[p]
        if p in stack:
            return None
        res = set()
        for n in requires.get(p, []):
            q = provides.get(n)
            if q is None:
                memo[p] = None
                return None
            sub = closure(q, stack + (p,))
            if sub is None:
                memo[p] = None
                return None
            res |= sub | {q}
        memo[p] = res
        return res

    pcm_dir = out / "pcm"
    pcm_dir.mkdir(exist_ok=True)
    (out / "obj").mkdir(exist_ok=True)
    (out / "lib").mkdir(exist_ok=True)

    def pcm_of(name):
        return pcm_dir / (name.replace(":", "-") + ".pcm")

    prov_name = {p: n for n, p in provides.items()}
    lines = ["rule hu\n  command = $cmd\n  description = HU $out\n  deps = gcc\n  depfile = $out.d\n",
             "rule pcm\n  command = $cmd\n  description = PCM $out\n  deps = gcc\n  depfile = $out.d\n",
             "rule cc\n  command = $cmd\n  description = CC $out\n  deps = gcc\n  depfile = $out.d\n",
             "rule obj\n  command = $cmd\n  description = OBJ $out\n",
             # response file: HAL alone has ~600 objects, past Windows' command-line limit.
             f"rule ar\n  command = \"{AR}\" rcs $out @$out.rsp\n"
             "  rspfile = $out.rsp\n  rspfile_content = $in\n  description = AR $out\n"]
    for h in hu.values():
        if h["pcm"].exists() or True:
            cmd = " ".join(f'"{c}"' if " " in str(c) else str(c) for c in hu_cmd(h))
            lines.append(f"build {ninja_path(h['pcm'])}: hu\n  cmd = {cmd}\n")

    def q(c):
        s = str(c)
        return f'"{s}"' if " " in s else s

    skipped = []
    objs = {t: [] for t in order}
    for t, p in allfiles:
        cl = closure(p)
        if cl is None or any(p is s for s, _ in scan_fail):
            skipped.append(p)
            continue
        info = tu[p]
        deps_pcm = [pcm_of(prov_name[q_]) for q_ in cl if q_ in prov_name]
        hu_keys = set(imports[p])
        for q_ in cl:
            hu_keys |= set(imports[q_])
        hu_keys = [k for k in hu_keys if k in ok_hu]
        hflags = hu_flags(hu_keys)
        hdeps = [hu[k]["pcm"] for k in hu_keys]
        common = [*flags, *info["near"], *forced_for(t), f"-fprebuilt-module-path={fwd(pcm_dir)}", *hflags]
        src = info["src"]
        implicit = " ".join(ninja_path(x) for x in deps_pcm + hdeps)
        implicit = f" | {implicit}" if implicit else ""
        obj = info["obj"]
        if info["is_ixx"] and p in prov_name:
            pcm = pcm_of(prov_name[p])
            cmd = " ".join(q(c) for c in [CLANGXX, *common, "-x", "c++-module", "--precompile", "-MD", "-MF", f"{fwd(pcm)}.d", src, "-o", pcm])
            lines.append(f"build {ninja_path(pcm)}: pcm {ninja_path(src)}{implicit}\n  cmd = {cmd}\n")
            cmd2 = " ".join(q(c) for c in [CLANGXX, *common, "-c", pcm, "-o", obj])
            lines.append(f"build {ninja_path(obj)}: obj {ninja_path(pcm)}\n  cmd = {cmd2}\n")
        else:
            extra = ["-x", "c++-module"] if info["is_ixx"] else []
            cmd = " ".join(q(c) for c in [CLANGXX, *common, *extra, "-MD", "-MF", f"{fwd(obj)}.d", "-c", src, "-o", obj])
            lines.append(f"build {ninja_path(obj)}: cc {ninja_path(src)}{implicit}\n  cmd = {cmd}\n")
        objs[t].append(obj)
    libs = []
    for t in order:
        if not objs[t] or t == "app":
            continue
        lib = out / "lib" / f"lib{t}.a"
        libs.append(lib)
        lines.append(f"build {ninja_path(lib)}: ar {' '.join(ninja_path(o) for o in objs[t])}\n")
    defaults = list(libs)
    if "app" in order and objs["app"]:
        # The entry object goes in directly (it holds JNI_OnLoad); the engine libraries are
        # archives, so only what the entry actually reaches is pulled in.
        so = out / "lib" / "libspectrum.so"
        lines.append(
            f'rule link\n  command = "{CLANGXX}" --target={ABI_TRIPLE}{API} -shared -fuse-ld=lld -static-libstdc++ '
            f'-Wl,-z,max-page-size=16384 -o $out @$out.rsp -Wl,--start-group $libs -Wl,--end-group '
            f'-llog -landroid -lvulkan\n  rspfile = $out.rsp\n  rspfile_content = $in\n  description = LINK $out\n')
        third_party = sorted((OUT / "vcpkg_installed" / "arm64-android" / "lib").glob("*.a"))   # see android/vcpkg.json
        lines.append(f"build {ninja_path(so)}: link {' '.join(ninja_path(o) for o in objs['app'])}"
                     f" | {' '.join(ninja_path(l) for l in libs)}\n"
                     f"  libs = {' '.join(ninja_path(l) for l in libs + third_party)}\n")
        defaults.append(so)
    if defaults:
        lines.append("default " + " ".join(ninja_path(l) for l in defaults) + "\n")
    (out / "build.ninja").write_text("\n".join(lines), encoding="utf-8")
    print(f"\n{len(allfiles) - len(skipped)} buildable, {len(skipped)} skipped; wrote {fwd(out / 'build.ninja')}")
    if args.no_build:
        return 0
    cmd = [str(NINJA), "-C", str(out), "-j", str(args.j)] + (["-k", "0"] if args.k else [])
    rc = subprocess.run(cmd).returncode
    if rc != 0:
        # The NDK's Clang crashes at random under heavy parallel load (access violation while
        # generating IR, e.g. for HAL.Debug.ixx.o); the same command succeeds when rerun.
        print("\nbuild failed; retrying once in case of a transient compiler crash...")
        rc = subprocess.run(cmd).returncode
    return rc


if __name__ == "__main__":
    sys.exit(main())
