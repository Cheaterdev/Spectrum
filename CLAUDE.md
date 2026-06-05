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
