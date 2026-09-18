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
- SIG shader-binding code generation (ANTLR4-driven, generates HLSL + C++ from a shared `.sig` DSL)

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

Large parts of the engine are generated rather than hand-written, and live under `autogen/` directories that are checked in but should not be edited by hand:
- **SIG → HLSL/C++**: `sources/SIGParser` (ANTLR4 grammar `SIG.g4`) parses `.sig` files into shared GPU/CPU structs and resource-binding tables, emitting HLSL and C++ into `sources/HAL/SIG/autogen/` and `sources/HAL/autogen/` (binding tables, PSOs, enums)
- **FrameGraph passes**: pass declarations, resource IDs, and context plumbing are generated into `sources/RenderSystem/FrameGraph/autogen/` (`pass_ids.h`, `resource_ids.h`, `pass_defaults.*`, `context_snapshot.cpp`, per-pass headers under `autogen/pass/`)
- **Work Graph nodes**: PSOs for DX12 Work Graph nodes are generated from Jinja templates (`workgraph_node_pso.jinja`, `workgraph_nodes.jinja`)

![img](https://cheater.dev/Spectrum.png)
https://cheater.dev
