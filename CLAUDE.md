# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

Spectrum uses **Sharpmake** (a C#-based build system) to generate Visual Studio 2026 project files, and **vcpkg** for dependency management.

### First-time setup
```bat
setup.bat          # Init Sharpmake submodule + build it, then generates VS projects
```

### Regenerate VS projects (after changing main.sharpmake.cs or adding/removing files)
```bat
generate_project.bat
```

### Regenerate code from .prism files
Tools → Regenerate Prism code in VS (Prism extension), or run `bin/profile/prismc.exe` from `sources/Prism/`.

### Changing the Prism grammar
The grammar is a cpp-peglib (PEG) string in `sources/Prism/PrismGrammar.h`; there is no generated parser, so a grammar edit
is just a rebuild of the Prism project. `peglib.h` (vcpkg port `cpp-peglib`) is included only by the plain, non-module
`sources/Prism/PrismBridge.cpp` (a header unit of it leaves its static string constants null), which hands `Parsing.cpp` a
small AST (`PrismAst.h`). Keywords are contextual (`!IDCHAR`-guarded tokens), so where a keyword-led alternative follows an
`ID`-led one, order them keyword first (see `rt_stat`, `value_id`). All PSO kinds share one `pso_block`; which statements
each kind takes is checked in `Parsing.cpp` (`PsoKind`), and PSO/node parameters are typed by name in the grammar.
`prismc --check a.prism` parses files without validating or generating anything, for trying grammar changes.

### Build
Open `projects/Spectrum.sln` in Visual Studio 2026. Three configurations: **Debug**, **Profile**, **Retail**.

Executables output to `bin/Debug`, `bin/Profile`, `bin/Retail`. The working directory for running is `workdir/`.

### Platforms (Android, later others)
Where things live: platform **source code** (entry point, stand-ins for APIs the platform lacks, packaging files) is in
`sources/Platform/<Name>/`, like `sources/HAL/API/<Backend>/` is for graphics backends; the **scripts** that build and run it are
`build/platforms/<name>/<command>.py`, started through `python build/driver.py <platform> <command> [args]`;
`build/clang_modules.py` is the platform-neutral part of a Clang modules build (Ninja helpers, change-only writes, the cached
dependency scan) for the next Clang platform to reuse. Generated output is `projects/<platform>/`. Windows is Sharpmake/MSBuild
only and has no driver. A new platform: a folder in `sources/Platform/`, a folder in `build/platforms/` with its commands, an entry in
`BuildPlatforms` / a solution in `main.sharpmake.cs`.

### Writing platform-specific code
**No `#ifdef _WIN32` / `__ANDROID__` / `SPECTRUM_PLATFORM_*` to pick between platforms** (and no "not `_WIN32`" meaning Android).
Platform differences live in folders named after the platform, and the build picks the folder:

- **Folder per platform inside the layer that needs it**: `sources/HAL/API/Vulkan/Windows/`, `.../Android/`,
  `sources/Core/Windows/`, `sources/Core/Android/`. The shared code calls a small function (or macro) with one
  definition per platform; it never branches on the platform itself. If a current function mixes shared logic with a
  platform part, split it so the platform part is its own function (e.g. the swapchain's `hal_create_window_surface` /
  `hal_query_window_extent`, defined in `HAL/API/Vulkan/<Platform>/HAL.Vulkan.Surface.cpp`).
- **Windows builds skip the other platforms' folders** (`BuildPlatforms.NonWindowsFolders` in `main.sharpmake.cs`, applied to
  every project through `SourceFilesBuildExcludeRegex`); **the Android build skips `Windows` folders**
  (`OTHER_PLATFORM_FOLDERS` in `build/platforms/android/build.py`) and compiles `Android` ones with everything else. A new
  platform folder name goes into both lists. New files need `generate_project.bat` (Sharpmake enumerates the sources).
- **Platform code that needs OS headers is a plain (non-module) `.cpp`** with `extern "C++"` declarations on the module side, like
  `png_bridge.cpp` and the surface files: `<windows.h>`, `vulkan_android.h`, `png.h` stay out of the module. On Android add such
  a file to `PLAIN_TU` in `build.py` if it includes `<vulkan/vulkan.h>` (otherwise the script rewrites the include).
- **Macros needed in the forced include** (`Core/Defines.h` must stay declaration-free, Clang rejects declarations ahead of
  `module X;`): a header with the same name in each platform folder (`Core/Windows/Defines.Platform.h`,
  `Core/Android/Defines.Platform.h`), found through the include path (Sharpmake adds `Core/Windows`, `build.py` adds
  `Core/Android`). `ASSERT` expands to `CORE_ASSERT_FAIL`, defined there; call `CORE_ASSERT_FAIL(msg, __FILE__, __LINE__)`
  for an unconditional fatal error instead of `::Core::assert_fail`.
- **First ask whether the difference has to exist.** Several ifdefs were just MSVC-only conveniences: `Exceptions` stores its
  own message and overrides `what()` on every platform; the unused Windows `m_fenceEvent` member in the Vulkan `Event` was
  deleted. A difference that disappears needs no folder.
- Not an ifdef problem, keep as is: feature tests (`__has_include`, `__cpp_*`), `HAL_BACKEND_*` (graphics API choice), and
  `DEV`/`LEAK_TEST_ENABLE` (build configuration).
- Still to migrate (use the folder approach when touching them): the `PLATFORM_RULES` text rewrite in `build.py` for
  `VK_USE_PLATFORM_WIN32_KHR` / `VK_KHR_WIN32_SURFACE_EXTENSION_NAME`, and the `SPECTRUM_PLATFORM_ANDROID` / `__ANDROID__` blocks in
  `crossguid.ixx`, `TextEngine.cpp` and the tests.
- Vulkan/STL headers are never `#include`d in module code: `import vulkan;` / `import stl.core;`. Macros and `static const` values a
  module cannot export are listed in `Modules/vulkan/vulkan_exports.inc` (generated by `gen_exports.py` there).

### Android
`generate_project.bat` also generates `projects/SpectrumAndroid.sln` (configs **Debug/Profile/Retail-Android**).
It holds one NMake project, `SpectrumAndroid`, which depends on `Tools` (so the Windows `spectrum_tool` and the engine
layers below it are built first, like the Windows solution). F7 then runs `driver.py android prepare` (arm64 vcpkg ports,
and the shader/texture/asset precompile with `spectrum_tool`, each skipped while its inputs are unchanged), `... build
--target app` and `... package` (APK in `projects/android/<mode>/spectrum.apk`); a clean `projects/` works from a
single F7. Clean/Rebuild wipe that mode's dir only (not the vcpkg install or the shader cache). F5 runs
`build/platforms/android/run.py` (install, launch, stream `logcat -s Spectrum`). `device.py` picks the phone: one connected
phone is used silently;
with none or several (or `--pick`) a small window opens to select, connect (ip:port) or pair (ip:port + code) one, and a paired
phone advertising over mDNS is connected automatically. `run.py` reinstalls (uninstall, then install) when the installed app
was signed by a different debug keystore. `spectrum_tool` has been seen to crash while shutting down after writing
everything (`assets` command); prepare.py accepts that only when the tool printed its success line. It cannot be an MSBuild/AGDE
build because AGDE has no C++ module / header-unit support; `build.py` does the real compiling.
**Debugging on the device:** run
`python build/driver.py android debug --mode debug -b SomeFunction` (a console lldb): it starts the NDK's `lldb-server` inside the debuggable app,
attaches the NDK `lldb` in a console, and loads the unstripped `projects/android/<mode>/lib/libspectrum.so` for
symbols. Source-level breakpoints, stepping, stacks and locals work in `debug` mode (built with full `-g`; `package.py`
strips the DWARF from the APK copy, the unstripped one stays in `lib/`); `profile` has line tables only (no locals).
Gotchas: `frame variable` on big objects crashes the NDK's Windows lldb at the default depth, so debug.py sets
`target.max-children-depth 1` (use `frame variable -D N x` for more); SIGSEGV/SIGBUS are passed to the app without
stopping because ART raises them itself (`--crashes` stops on ones from our library, not fully reliable).

**Editor breakpoints (VS Code):** Visual Studio's own Android attach needs an AGDE project as the startup project (not
possible here), so use VS Code with the CodeLLDB extension: open the repo folder, pick "Spectrum Android (debug, full
symbols)" in Run and Debug, F5. `.vscode/tasks.json` runs `debug.py --server --launch` (app + lldb-server, writes
`projects/android/<mode>/attach.lldb` with the pid) and `.vscode/launch.json` replays the same lldb commands as `debug.py`.
Gutter breakpoints stay pending until `target symbols add` loads the unstripped library after attach.

**Tests on the phone:** `python build/driver.py android test [--mode debug] [--filter Name]` installs the APK, pushes
`workdir/test_references` (the same golden PNGs as Windows; PNG encode/decode on Android is libpng via
`sources/Platform/Android/png_bridge.cpp` + `HAL.Android.TextureData.cpp`), runs the app in test mode and prints failures;
failing images (`*_actual.png`, `*_diff.png`) and the full log land in `projects/android/test_results/` and `test_log.txt`.
On Android a missing reference is a failure, not "save and pass". Per-platform references are not used yet: the
tests share the Windows ones and the default tolerance (2 per channel) absorbs GPU rounding.
**Test shaders are files, not strings:** a phone has no shader compiler, so every shader a test uses must be a PSO declared in
Prism (`sources/Prism/defs/gpu_tests.prism`, HLSL in `workdir/shaders/test/`) and used as `gfx.set_pipeline<PSOS::Tests::Name>()`,
so `spectrum_tool shaders` precompiles it with the rest. An inline-HLSL `SimpleGraphicsPSO` in a test builds an empty pipeline
on the device, and the draw is silently skipped (no error): the symptom is a render of just the clear colour.

Same commands from a shell: `python build/driver.py android build --mode profile --target app`,
`... android package --mode profile --install`, `... android run --mode profile`.
`generate_project.bat` also writes `projects/android/targets.json` (the engine layer stack, from `EngineLayers` in
`main.sharpmake.cs`, which the Windows projects use too) and `projects/android/vcpkg.json` + `vcpkg-configuration.json` (the
arm64-android subset of the root manifest; the port list is `AndroidMetadata.VcpkgPorts`). `build.py` needs targets.json, so
run `generate_project.bat` first on a fresh checkout (`prepare.py` runs the vcpkg install, ~4 min the first time).
`build.py` caches
dependency scans in `projects/android/<mode>/scan_cache.json` (a no-change build takes ~8 s).

### Kill MSBuild after every build
After any `msbuild` invocation you run, always kill the lingering worker
(`/nodeReuse:true` keeps `MSBuild.exe` alive after the build). It locks files,
blocks Visual Studio / AGDE / other installers, and often prevents Claude Code
from being restarted. Pass `-nodeReuse:false` when building, and also run:
```powershell
Get-Process MSBuild,VBCSCompiler -ErrorAction SilentlyContinue | Stop-Process -Force
```

## Architecture

### Layer Stack (bottom to top)
```
Modules       — C++ module wrappers for third-party libraries (assimp, cereal, etc.)
Core          — Engine foundation: math, serialization, threads, events, filesystem, profiling
HAL           — Hardware Abstraction Layer over D3D12: device, queues, resources, descriptors, shaders
RenderSystem  — High-level rendering: FrameGraph, materials, lighting, GUI, scene, assets
Spectrum      — Application entry point, ties everything together
```

Each layer has a `Defines.h` that chains upward (e.g. `RenderSystem/Defines.h` includes `HAL/Defines.h`). Every `.cpp`/`.ixx` in a project gets this forced-included automatically via Sharpmake.

### C++ Modules (.ixx)
The codebase uses **C++23 modules** (`.ixx` files) extensively. Each subsystem exposes a module interface — for example `HAL.Device.ixx` exports `module HAL:Device`. Headers (`.h`) are used for things that can't be modules (forced includes, third-party interop).

### Prism
Prism is the engine's declaration language: `.prism` files in `sources/Prism/defs/` declare shared GPU/CPU structs and their resource bindings, PSOs, raytracing PSOs, FrameGraph passes, pipelines, enums, constants and HLSL helper functions. `prismc` (a cpp-peglib-based compiler, `sources/Prism/`) generates HLSL and C++ from them into `sources/HAL/autogen/`, `sources/RenderSystem/FrameGraph/autogen/` and `workdir/shaders/autogen/`. The language was called SIG (shader input groups) before it grew beyond bindings; the engine-side binding code keeps that name (`HAL/SIG/`, `SlotID`, `Table::`).

### FrameGraph
The `RenderSystem/FrameGraph` subsystem manages render pass scheduling, automatic resource transitions, and async compute. Render passes declare their resource reads/writes; the FrameGraph resolves barriers and execution order.

### Modules Layer
`sources/Modules/` wraps third-party vcpkg libraries as C++ modules (`.ixx`). Each subfolder (`assimp/`, `cereal/`, etc.) contains an `.ixx` that re-exports the library through the module system.

## Key Files
- `main.sharpmake.cs` — all project/solution configuration; edit this to add files, dependencies, or defines
- `vcpkg.json` — vcpkg dependency manifest
- `vcpkg-configuration.json` — vcpkg registry config (uses git registry + custom overlay in `custom-overlay/`)
- `workdir/` — runtime working directory (shaders, assets); not fully committed (see .gitignore)

## CI/CD
GitHub Actions workflow at `.github/workflows/build.yml` builds the **Retail** configuration on `windows-2025-vs2026` runners. vcpkg dependencies are cached in `vcpkg_installed/`. Build artifacts (workdir output) are uploaded after a successful build.

## Runtime-tunable settings (`Variable<T>`)

Any bool/float/enum knob that should be inspectable or editable at runtime —
an A/B toggle, a debug visualization switch, a tunable threshold — goes
through `Variable<T>` (`sources/Core/Tree/Tree.ixx`), **not** a hand-wired
`check_box_text`/`float_slider`/`edit_text` row built directly in `main.cpp`
or wherever the class lives. The hand-wired version was the actual shape VSM
and `triangle_drawer::auto_rotate_sun` used to take — nine near-identical
7-line blocks in `main.cpp` for VSM alone — before being migrated to this
system; don't reintroduce that pattern for a new toggle.

```cpp
class stencil_renderer : public GUI::base, public VariableContext
{
public:
    Variable<bool>  draw_aabb        = { false, "Draw AABB", this };
    Variable<float> falloff          = { 1.0f, "Falloff", this, 0.0f, 5.0f };   // constrained -> slider
    Variable<float> arbitrary_offset = { 0.0f, "Offset", this };                // unconstrained -> free-form text box
    Variable<Mode>  mode             = { Mode::Fast, "Mode", this };            // enum -> combo box (magic_enum-driven)
};
```

Declaring a `Variable<T>` member is the entire integration: it self-registers
with its owning `VariableContext` and shows up automatically in the app's
Properties debug panel (`ParameterWindow.ixx` picks the widget type — slider,
checkbox, text box, or combo box — from `T` and whether a range was given),
already wired with a "Reset" button and live sync if the value changes from
somewhere other than that widget (see `get_default()`/`is_default()`/
`reset_to_default()`, and the `on_change`/`on_changed` events on
`Variable<T>`/`VariableBase`). None of that needs to be built by hand again.

The owning class must derive `VariableContext` (as above) to get an implicit
`this` for the Variable's third constructor argument. If it can't (e.g. it
already derives `GUI::base`, whose own `add_child` would become ambiguous
with `VariableContext`'s), use `VariableContext::create(L"name")` to obtain
an owned `std::unique_ptr<VariableContext>` member instead, and pass `*that`.

Every `VariableContext` of a given class registers under the same fixed
name (e.g. every `mesh_renderer` is named `"mesh_renderer"`), so if a class
is instantiated more than once, otherwise-identical entries pile up as flat,
indistinguishable siblings in the Properties tree. Disambiguate by wrapping
construction in `VariableContext::Scope scope(some_persistent_context);` —
subsequently-constructed named contexts nest under `some_persistent_context`
instead of the global root. The context passed to `Scope` must outlive the
`Scope` itself (a base-class `this`, or a `create()`-obtained member) — see
the comments on `VariableContext::Scope` in `Tree.ixx` for why a Scope owning
a temporary context is wrong.

## Profiling

`PROFILE(L"name")` (CPU) and `PROFILE_GPU(L"name")` (GPU) are scoped timers defined in `sources/Core/Profiling/macros.h`, forced-included everywhere via `Core/Defines.h`. They compile to `((void)0)` unless `PROFILING` is defined for the build config, so they're free to add liberally — no need to gate them behind anything yourself.

When writing or editing any function that does non-trivial per-frame CPU work (loops over resources/passes, resource-view creation, upload/copy calls) or issues GPU commands (draws, dispatches, barrier-heavy resource binding), add scoped `PROFILE` blocks around its logically distinct sub-sections, not just one at the top of the function:

```cpp
{
    PROFILE(L"vsm_hiz_copy");
    // ... the copy dispatch ...
}
{
    PROFILE(L"vsm_hiz_downsample");
    // ... the downsample loop ...
}
```

- Scope with an explicit `{ }` block — the macro declares a local variable whose destructor stops the timer at end of scope, so an unbracketed `PROFILE(...)` at the top of a big function silently times everything after it, not just the intended section.
- Name markers by what they measure, not where they live (`vsm_hiz_copy`, not `loop1`) — this is what shows up in the profiler UI and needs to be legible next to unrelated markers from other systems.
- Prefer several small, named scopes over one broad one when a function has multiple distinct phases (setup vs. dispatch vs. teardown) — this is what makes a profiler capture actionable instead of just "this function is slow somewhere."
- This is the same convention already used throughout `HAL.CommandList.cpp` (`commit_tables`, `transitions`, `rt_transitions`) and `MipMapGeneration.cpp` — match that granularity when adding new instrumented code, don't invent a different style.

## Temporary debug logging

Debug logging added to chase a specific problem is scaffolding, not code. It
exists for one investigation and becomes noise the moment that investigation
ends — so it needs to be obvious on sight and trivial to remove.

**Write temporary log output to files with a `.temp` extension**, never into
`log.txt` or the engine's normal logging path. `*.temp` is gitignored, so
scratch output can never be committed by accident, and the extension makes
every temporary artifact greppable as a set rather than something you have to
remember file by file.

**When a debugging session ends, ask before cleaning up.** Once the bug is
found and fixed, list the temporary logging that was added — the call sites and
the `.temp` files — and ask whether to remove it. Don't strip it automatically:
a fix often needs one more round of verification, and silently deleting the
instrumentation that proved it means rebuilding it from scratch. Equally, don't
leave it behind unmentioned — untracked `PROFILE`-adjacent spam and stray
writes accumulate and are hard to attribute later.

This is specifically about *temporary* instrumentation. Permanent diagnostics
that earn their place — the VSM active-window diag, the D3D12 message callback
— are normal code and stay.

## Visual confirmation

A clean build and 0 D3D12 validation errors prove the pipeline runs without
crashing — they say nothing about whether a denoiser actually denoises, a
shadow looks right, or a debug view shows what it's supposed to. Anything
whose correctness is a visual judgment call, not a log-checkable fact, is not
something to verify by guessing, screenshotting blind, or declaring success
off the log alone.

When a change needs that kind of check, stop and ask the user to look at it
themselves — say exactly which toggle(s)/GUI setting(s) to flip and to what
value (e.g. "Shadow Src → RTX Ref"), and what to look for. Don't flip runtime
defaults, rebuild, and run the app repeatedly to eyeball it in their place.

## Status reports

End every report on a piece of work with these five sections, in this order,
each under its own heading and separated by a horizontal rule (`---`):

1. **Summary** — one or two sentences: where things stand.
2. **Done** — what changed, and how it was verified (build, tests, runs,
   validation). Unverified work goes under Not done, not here.
3. **Not done** — what failed, was skipped, is still broken, or turned out
   different from what was expected.
4. **Planned** — the next steps, in the order they would happen.
5. **Action needed from you** — decisions to make, things to look at (with
   exact toggles, per Visual confirmation), manual steps only the user can do.
   A decision lists its options and marks the recommended one, with the reason
   in a clause.

Keep every section, writing "None." when one is empty — an absent section is
ambiguous, an empty one is not. The sections are for reports on work; a short
answer to a question needs none of them.

## Editing files

Use the editor's native file-edit tool for changes, not a script (Python,
sed, etc.) that reads a file and writes back a modified copy. The one
exception is a genuine multiple-occurrence replacement (renaming a symbol
throughout a file, etc.) — the native tool's own "replace all" mode is still
preferred there; reach for a script only when that isn't enough.

## Comments

Comment sparingly. Prefer code that doesn't need explaining, and don't restate
in prose what the line below already says: `// increment the counter` above
`++counter` is pure cost, and comments that merely narrate drift out of sync
with the code and start actively lying.

Write a comment when the reader would otherwise be right to think the code is
wrong:

- A workaround for a driver, API, or compiler bug — say which, and what breaks
  without it.
- A non-obvious ordering or lifetime requirement — why this call must precede
  that one.
- A deliberate deviation from the obvious implementation, with the reason.
- A constant whose value came from measurement or a spec, not from choice.

The test is whether someone competent would be tempted to "simplify" the code
and break it. If yes, explain why it is the way it is. If no, leave it alone.

## FrameGraph: A/B resource selection

When a pass reads one of several alternative resources chosen at runtime — a
debug view source, a ping-pong history buffer, a denoised vs. raw input — make
the selection in the **setup** function too, not only where the resource is
bound for rendering.

Setup is what links a resource to the pass. Calling `builder.need()` on both
sides of a choice declares a dependency on the alternative that will never be
read, and the graph then has to honour it: the unused resource is kept alive
across this pass, and the transitions compiled for it are barriers paid for
nothing. Worse, the edge is invisible at the call site that actually matters —
the render function looks correct because it binds only one of them.

Resolve the choice once into a single handle, then use that same handle in both
places:

```cpp
// selection resolved once, stored on the context
ui_ctx.result_texture_handler = Handlers::Texture(debug_source(dbg.mode));

// setup: link only what was selected
if (builder.exists(ui_ctx.result_texture_handler))
    builder.need(ui_ctx.result_texture_handler, ResourceFlags::PixelRead);

// render: bind the same handle
if (ui_ctx.result_texture_handler)
    c.result_texture_srv = *ui_ctx.result_texture_handler;
```

- Guard with `builder.exists()` before `need()` — the selected resource may not
  be produced at all in the current graph configuration, and a hard `need()` on
  a resource no pass writes is a graph-build failure rather than a missing
  input.
- Resolve the selection *before* setup runs (in graph construction) rather than
  recomputing the branch in both functions. Two copies of the same condition
  drift, and when they disagree the pass links one resource and binds another —
  which surfaces as a binding that reads whatever the unused resource happened
  to contain, not as an error.
- `UI_Render` in `sources/RenderSystem/GUI/Base.cpp` is the reference: the
  `DebugMode` switch picks the source once in `create_graph`, and both the setup
  and render sides use only `result_texture_handler`.
