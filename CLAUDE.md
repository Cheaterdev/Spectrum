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

### Regenerate SIG parser (after changing .sig files or SIG.g4 grammar)
```bat
generate_sigs.bat
```

### Build
Open `projects/Spectrum.sln` in Visual Studio 2026. Three configurations: **Debug**, **Profile**, **Retail**.

Executables output to `bin/Debug`, `bin/Profile`, `bin/Retail`. The working directory for running is `workdir/`.

## Architecture

### Layer Stack (bottom to top)
```
Modules       — C++ module wrappers for third-party libraries (assimp, cereal, antlr4, etc.)
Core          — Engine foundation: math, serialization, threads, events, filesystem, profiling
HAL           — Hardware Abstraction Layer over D3D12: device, queues, resources, descriptors, shaders
RenderSystem  — High-level rendering: FrameGraph, materials, lighting, GUI, scene, assets
Spectrum      — Application entry point, ties everything together
```

Each layer has a `Defines.h` that chains upward (e.g. `RenderSystem/Defines.h` includes `HAL/Defines.h`). Every `.cpp`/`.ixx` in a project gets this forced-included automatically via Sharpmake.

### C++ Modules (.ixx)
The codebase uses **C++23 modules** (`.ixx` files) extensively. Each subsystem exposes a module interface — for example `HAL.Device.ixx` exports `module HAL:Device`. Headers (`.h`) are used for things that can't be modules (forced includes, third-party interop).

### SIG System
`.sig` files define shared GPU/CPU data structures and resource bindings. `SIGParser` (an ANTLR4-based tool) parses them and generates HLSL and C++ binding code. SIG files live in `sources/SIGParser/sigs/`. Generated output goes to `sources/HAL/SIG/autogen/` and `sources/RenderSystem/FrameGraph/autogen/`.

### FrameGraph
The `RenderSystem/FrameGraph` subsystem manages render pass scheduling, automatic resource transitions, and async compute. Render passes declare their resource reads/writes; the FrameGraph resolves barriers and execution order.

### Modules Layer
`sources/Modules/` wraps third-party vcpkg libraries as C++ modules (`.ixx`). Each subfolder (`assimp/`, `cereal/`, `antlr4/`, etc.) contains an `.ixx` that re-exports the library through the module system.

## Key Files
- `main.sharpmake.cs` — all project/solution configuration; edit this to add files, dependencies, or defines
- `vcpkg.json` — vcpkg dependency manifest
- `vcpkg-configuration.json` — vcpkg registry config (uses git registry + custom overlay in `custom-overlay/`)
- `workdir/` — runtime working directory (shaders, assets); not fully committed (see .gitignore)

## CI/CD
GitHub Actions workflow at `.github/workflows/build.yml` builds the **Retail** configuration on `windows-2025-vs2026` runners. vcpkg dependencies are cached in `vcpkg_installed/`. Build artifacts (workdir output) are uploaded after a successful build.

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
