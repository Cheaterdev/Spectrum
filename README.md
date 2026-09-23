[![Build](https://img.shields.io/github/actions/workflow/status/Cheaterdev/Spectrum/build.yml?job=build&label=build)](https://github.com/Cheaterdev/Spectrum/actions/workflows/build.yml)
[![Tests](https://img.shields.io/github/actions/workflow/status/Cheaterdev/Spectrum/build.yml?job=test&label=tests)](https://github.com/Cheaterdev/Spectrum/actions/workflows/build.yml)

# Spectrum


## Features

**Rendering backend**
- DX12
- Vulkan backend (WIP)
- Async Compute
- DX12 Work Graphs
- GPU meshlet generation / mesh shader pipeline
- DirectStorage-accelerated asset streaming

**FrameGraph**
- Automatic resource transitions
- Easy resource binding
- GBuffer deferred rendering pipeline
- GPU Occlusion Culling
- Virtual/tiled texture streaming (sparse-residency GPU virtual texturing)

**Ray tracing & GI**
- DXR
- Raytraced shadows
- Raytraced reflections
- Sparse Voxel GI with "infinite" bounces
- NRD denoising (REBLUR + SIGMA)
- Virtual Shadow Maps (VSM) and cascaded shadow maps (PSSM)

**Upscaling & anti-aliasing**
- DLSS / DLSS Ray Reconstruction (via NVIDIA Streamline)
- AMD FSR upscaling
- SMAA antialiasing

**Assets & content**
- Asset System with hot-reload file watching for shaders/assets
- Assimp mesh import, plus a custom RW4 importer
- Material Graphs with live preview
- Font/text rendering (FreeType-based)

**Tools & engine infrastructure**
- GUI
- Runtime-tunable properties (`Variable<T>`) with an in-app debug panel
- CPU/GPU profiling (scoped `PROFILE`/`PROFILE_GPU` markers)
- FrameGraph live debugger: pass/resource inspector, GPU pass timeline with barrier visualization, and a pannable/zoomable resource preview (2D/array/3D/cube textures and buffers)
- Prism, a declaration language for GPU/CPU structs, bindings, PSOs and FrameGraph passes (ANTLR4-driven `prismc` generates HLSL + C++ from `.prism` files), with a Visual Studio extension for highlighting, live diagnostics and navigation

## Build system

- [Sharpmake](https://github.com/ubisoft/Sharpmake) (C#-based) generates Visual Studio 2026 project files from `main.sharpmake.cs`
- [vcpkg](https://vcpkg.io) manifest-mode dependency management (`vcpkg.json`), with a custom overlay registry
- Debug / Profile / Retail configurations
- CI on GitHub Actions (`windows-2025-vs2026` runners) builds Retail and uploads `workdir` artifacts

## C++

- C++23, built extensively around **C++ modules** (`.ixx` module interfaces) rather than classic headers
- Layered module/include architecture (Modules → Core → HAL → RenderSystem → Spectrum), each layer force-including the one below via a chained `Defines.h`
- Third-party libraries (assimp, cereal, antlr4, DirectXTex, DirectStorage, freetype, jinja2cpp, ...) wrapped as C++ modules under `sources/Modules/`

## Code generation

Large parts of the engine are generated rather than hand-written. The source of truth is **Prism**, the engine's declaration language: `.prism` files in `sources/Prism/defs/` declare GPU/CPU shared structs and their slot bindings, root layouts, graphics/compute/raytracing/work-graph PSOs, FrameGraph passes and pipelines, enums, constants and HLSL helper functions. The `prismc` compiler (`sources/Prism/`, ANTLR4 grammar `Prism.g4`, Jinja2 templates) validates them and generates, into `autogen/` directories that are checked in but never edited by hand:
- **HAL** (`sources/HAL/autogen/`): binding tables and slots, root layouts, PSOs with permutation keys, raytracing PSOs, enums, constants
- **FrameGraph** (`sources/RenderSystem/FrameGraph/autogen/`): per-pass headers with generated setup (resource creation/needs, enable conditions), pipelines, `pass_ids.h`, `resource_ids.h`, `pass_defaults.*`, context dependency tables
- **HLSL** (`workdir/shaders/autogen/`): the matching shader-side structs, layouts, raytracing and work-graph node headers

Regenerate after editing a `.prism` file with **Tools → Regenerate Prism code** in Visual Studio, or `cd sources/Prism && ../../bin/profile/prismc.exe`; run `generate_project.bat` when generated files were added or removed. A mistake in a `.prism` file stops generation with `file(line,col): error:` messages and writes nothing.

The Visual Studio extension (`bin/editor/prism.vsix`, built by `sources/Prism/editor/gen_vs_extension.py`) adds highlighting, live errors with quick fixes, go-to-definition, hover, completion, outline and the regenerate command.

Language reference for developers: [`overview/Prism.txt`](overview/Prism.txt).

![img](https://cheater.dev/Spectrum.png)
https://cheater.dev
