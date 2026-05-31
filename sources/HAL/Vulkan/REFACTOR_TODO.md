# Vulkan Backend — Clean-Refactor TODO

This file records the work deferred while taking the **fast stub path** to get a
compiling Vulkan backend (C++20 module partition swap, no `#ifdef`s). The goal
of this document is so the "do it properly" pass can be done later without
re-deriving the analysis.

---

## 0. Architecture recap

`sources/HAL/D3D12/` and `sources/HAL/Vulkan/` both export the **same** module
partition names (`HAL:API.Device`, `HAL:API.Resource`, `HAL:Device`,
`HAL:Resource`, `HAL:Format`, …). Sharpmake compiles exactly one folder per
build via `target.Backend` (see `main.sharpmake.cs`, `HAL::ConfigureAll`).
Common files in `sources/HAL/*.cpp` compile in **both** backends and only call
into the `HAL::API::*` seam.

Partition ownership map (who defines what), discovered during scaffolding:

| Partition | Common file (both) | D3D12-only file | Vulkan-only file |
|---|---|---|---|
| `HAL:Device` | `HAL.Device.cpp` (singleton/managers) | `D3D12/HAL.D3D12.Device.cpp` (API::Device + get_texture_layout/compress) | `Vulkan/HAL.Vulkan.Device.cpp` |
| `HAL:Resource` | `HAL.Resource.cpp` (create_resource, getters) | `D3D12/HAL.D3D12.Resource.cpp` | `Vulkan/HAL.Vulkan.Resource.cpp` |
| `HAL:Resource.Buffer` | `HAL.Resource.Buffer.cpp` (init/read/write/ctors) | `D3D12/...Resource.Buffer.cpp` (cpu_data, dtor, to_native addr) | `Vulkan/...Resource.Buffer.cpp` |
| `HAL:DescriptorHeap` | `HAL.DescriptorHeap.cpp` (Handle/Storage/Factory) | `D3D12/...DescriptorHeap.cpp` (Descriptor::place, get_cpu/gpu) | `Vulkan/...DescriptorHeap.cpp` |
| `HAL:Heap` | `HAL.Heap.cpp` (get_type/size/as_buffer) | `D3D12/...Heap.cpp` (ctor, API::Heap) | `Vulkan/...Heap.cpp` |
| `HAL:Format` | `HAL.Format.cpp` (ctor/basic) | `D3D12/HAL.Format.cpp` (size, surface_info, …) | `Vulkan/HAL.Vulkan.Format.cpp` |
| `HAL:Queue` | `HAL.Queue.cpp` (orchestration) | `D3D12/...Queue.cpp` (API::Queue, DirectStorageQueue, tile maps) | `Vulkan/...Queue.cpp` |
| `HAL:CommandList` | `HAL.CommandList.cpp` + `HAL.CommandListRecorder.cpp` (ALL wrapper orchestration: GraphicsContext, ComputeContext, CopyContext, Transitions, Eventer, DelayedCommandList) | — | — |
| `HAL:API.CommandList` | — | `D3D12/...CommandList.cpp` | `Vulkan/...CommandList.cpp` |
| `HAL:PipelineState` | `HAL.PipelineState.cpp` (cache/desc) | `D3D12/...PipelineState.cpp` (on_change builders) | `Vulkan/...PipelineState.cpp` |
| `HAL:TextureData` | — | `D3D12/...TextureData.cpp` | `Vulkan/...TextureData.cpp` |
| `HAL:TiledMemoryManager` | `HAL.TiledMemoryManager.cpp` (tile logic) | `D3D12/...TiledMemoryManager.cpp` (init_tilings) | `Vulkan/...TiledMemoryManager.cpp` |
| `HAL:SwapChain` | `HAL.Swapchain.cpp` (getters/wait) | `DXGI/HAL.DXGI.Swapchain.cpp` (ctor/present/resize) | `Vulkan/...Swapchain.cpp` |
| `HAL:Adapter` | — | `DXGI/HAL.Adapter.cpp` | `Vulkan/...Adapter.cpp` |
| `HAL:Utils` | — | `D3D12/HAL.Utils.cpp` | `Vulkan/HAL.Vulkan.Utils.cpp` |
| `HAL:Impl` | — | `D3D12/HAL.Impl.cpp` | `Vulkan/HAL.Impl.cpp` |

**Key takeaway:** the entire CommandList *wrapper* layer is already
backend-agnostic — it records lambdas into `DelayedCommandList` and replays them
against `API::CommandList`. So Vulkan only ever needs to implement the
`API::CommandList` method bodies. No duplication of GraphicsContext/Transitions.

---

## 1. Stubs that still need real implementations (functional gaps)

These compile but do nothing yet. Ordered by milestone.

### Phase 1 — Device + adapter
- `Vulkan/HAL.Vulkan.Device.cpp` `API::Device::init`: real `vkCreateInstance`
  (+ `VK_LAYER_KHRONOS_validation` in debug), `VK_EXT_debug_utils` messenger,
  `vkCreateDevice` with extensions (swapchain, dynamic_rendering,
  synchronization2, buffer_device_address, descriptor_indexing,
  timeline_semaphore), `vmaCreateAllocator`. Fill `DeviceProperties`.
- `Vulkan/HAL.Vulkan.Adapter.cpp`: already enumerates real
  `VkPhysicalDevice`s; verify `Adapters::set_instance()` is called after
  instance creation (currently the instance lives on Device, but Adapters is a
  separate singleton — wire them, or move enumeration to use a temporary
  instance).  **Open design point**, see §3.
- `Vulkan/HAL.Vulkan.Device.cpp` `get_alloc_info`: use
  `vkGetImageMemoryRequirements` / buffer requirements instead of the
  size-only placeholder.

### Phase 2 — Swapchain
- `Vulkan/HAL.Vulkan.Swapchain.cpp`: surface, swapchain, image views,
  backbuffer wrap via `API::NativeImportHandle{ image, view, format }`,
  per-frame semaphores.

### Phase 3 — Command + clear
- `Vulkan/HAL.Vulkan.CommandList.cpp`: real `begin/end`, `transitions()` →
  `vkCmdPipelineBarrier2KHR` (use `to_native_stage`/`to_native_access`/
  `to_native(TextureLayout)` from Utils), clear via `vkCmdBeginRenderingKHR` +
  clear + `vkCmdEndRenderingKHR`, copies.
- `Vulkan/HAL.Vulkan.Queue.cpp` `API::Queue`: `vkGetDeviceQueue`, per-frame
  `VkCommandPool`, `vkQueueSubmit2`, timeline-semaphore signal/wait.
- `Vulkan/HAL.Vulkan.CommandAllocator.cpp`: `vkCreateCommandPool` /
  `vkResetCommandPool`.
- `Vulkan/HAL.Vulkan.Fence.cpp`: already implements real timeline-semaphore
  signal/wait — verify against the submit path.

### Phase 1+ — Resources / memory
- `Vulkan/HAL.Vulkan.Resource.cpp`: `vmaCreateBuffer` / `vmaCreateImage`,
  `vkGetBufferDeviceAddress`, persistent map for UPLOAD/READBACK, debug names.
- `Vulkan/HAL.Vulkan.Heap.cpp`: VMA-backed block allocation + placed
  sub-resources; map cpu_address for UPLOAD/READBACK.
- `Vulkan/HAL.Vulkan.QueryHeap.cpp`: `vkCreateQueryPool(TIMESTAMP)`.

### Phase 4 — Descriptors / pipelines / shaders
- `Vulkan/HAL.Vulkan.DescriptorHeap.cpp`: `VkDescriptorPool` / sets;
  `Descriptor::place(*)` writes; **bindless** via `VK_EXT_descriptor_indexing`.
- `Vulkan/HAL.Vulkan.RootSignature.cpp`: `VkDescriptorSetLayout`(s) +
  push constants → `vkCreatePipelineLayout`.
- `Vulkan/HAL.Vulkan.PipelineState.cpp`: `vkCreateGraphicsPipelines` /
  `vkCreateComputePipelines`; `VkPipelineCache` for `get_cache()`.
- HLSL → SPIR-V: DXC already in the project; add `-spirv` path (the DXC/
  folder compiles in **both** backends).
- `Vulkan/HAL.Vulkan.IndirectCommand.cpp`: real indirect buffer layout +
  `vkCmdDrawIndexedIndirect` / `vkCmdDispatchIndirect`.
- `Vulkan/HAL.Vulkan.TextureData.cpp`: real image decode (DirectXTex decode is
  API-agnostic and can be reused) + BC compression.

### Post-MVP
- Tiled/sparse: `Vulkan/HAL.Vulkan.TiledMemoryManager.cpp` `init_tilings` +
  `Queue::update_tile_mappings` via `vkQueueBindSparse`.
- Raytracing: `StateObject` / `dispatch_rays` / `build_ras` via
  `VK_KHR_acceleration_structure` + `VK_KHR_ray_tracing_pipeline`.
- Work graphs: no direct Vulkan equivalent — emulate or leave disabled.
- DirectStorage streaming (`DirectStorageQueue::execute`): replace with a
  Vulkan staging-buffer uploader (+ optional GDeflate).

---

## 2. Compatibility shims that should be removed in the clean version

The fast path introduced D3D12-named stand-ins so common files compile
unchanged. The *clean* refactor should replace these with backend-neutral
names in the common headers and drop the shims.

- `HAL.Vulkan.Utils.ixx` defines stub `D3D12_CPU_DESCRIPTOR_HANDLE`,
  `D3D12_GPU_DESCRIPTOR_HANDLE`, `DXGI_ADAPTER_DESC`,
  `D3D12_DISPATCH_ARGUMENTS`, `D3D12_DRAW_INDEXED_ARGUMENTS`,
  `D3D12_DISPATCH_MESH_ARGUMENTS`, `D3D12_PROGRAM_IDENTIFIER`.
  - Source leaks to fix in **common** code so the shims can die:
    - `HAL.DescriptorHeap.ixx` — `Handle::get_cpu()/get_gpu()` return
      `D3D12_CPU/GPU_DESCRIPTOR_HANDLE`. Introduce a neutral
      `HAL::DescriptorPointer { uint64 cpu; uint64 gpu; }` (or opaque) and
      change the common signature; each backend fills it.
    - `HAL.Device.cpp` — logs `adapter->get_desc().Description`. Introduce a
      neutral `HAL::AdapterInfo { std::wstring name; uint vendor, device; size_t vram; }`
      returned by `Adapter::get_info()`, and switch the log + the "Basic"
      device-selection heuristic to it.
    - `API::StateObject::id` is `D3D12_PROGRAM_IDENTIFIER` (work-graph only) —
      gate behind a neutral type once work-graphs are abstracted.

- `to_native(const ResourceAddress&)` is declared in `HAL.Vulkan.Utils.ixx`
  and defined in `HAL.Vulkan.Resource.Buffer.cpp` to mirror the D3D12 global.
  In the clean version, make `ResourceAddress::get_native()` (or similar) a
  first-class HAL method instead of a free `to_native`.

---

## 3. Open design points

- **Adapter/instance ownership.** D3D12 has a global `DXGI::Factory` in the
  `Adapters` singleton that can enumerate before any device. Vulkan needs a
  `VkInstance` first. Options: (a) `Adapters` creates its own lightweight
  instance for enumeration; (b) Device creates the instance and pushes it to
  `Adapters::set_instance()` before enumerating. Current scaffold leans toward
  (b) but `Device::create_singleton()` (common) calls
  `Adapters::get().enumerate()` *before* constructing a Device — so (a) is
  probably required. **Resolve before Phase 1.**

- **`HAL::init()`** (`HAL.Impl.cpp`): D3D12 enables the debug layer globally
  before device creation; Vulkan validation is per-instance. The Vulkan
  `init()` currently just creates the `Adapters` singleton. Decide where the
  instance is born (ties into the point above).

- **Backend-neutral barrier types already exist** (`BarrierSync`,
  `BarrierAccess`, `TextureLayout`) and map cleanly to sync2 — no shim needed,
  this is the clean seam to imitate elsewhere.

---

## 4. Build-system notes (`main.sharpmake.cs`)

- `Backend` fragment added (`D3D12 | Vulkan`); solution configs are
  `Debug-D3D12`, `Debug-Vulkan`, etc.
- `HAL::ConfigureAll` excludes the other backend folder via
  `SourceFilesBuildExcludeRegex`; DXC/ stays in both.
- `Modules::ConfigureAll` excludes the other backend's module wrapper
  (`Modules/d3d12/` vs `Modules/vulkan/`).
- `vcpkg.json` adds `vulkan-memory-allocator` + `vulkan-headers`.
- Vulkan build defines `HAL_BACKEND_VULKAN` (currently unused by source — keep
  it ifdef-free; it exists only for tooling/diagnostics).

---

## 5. When doing the clean version

1. Introduce the neutral types in §2 in the **common** headers.
2. Update the handful of common call sites (DescriptorHeap handle, Device log).
3. Delete the D3D12-named shims from both `HAL.Vulkan.Utils.ixx` and the D3D12
   Utils (replace with the neutral types there too).
4. Keep the partition-swap file layout — it is the correct long-term structure;
   only the *contents* of the stubs change.
