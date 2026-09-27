# Vulkan Binding Model + Optional RT / Mesh Shaders — Plan

Goal: a Vulkan resource-binding model that runs on Android, plus making
raytracing and mesh shaders optional engine features (their passes are skipped
when unsupported; fallback passes come later).

Status: **Track B done; Track A implemented** (Vulkan test.exe 220/222, 0
failed). Replaces the earlier `VK_EXT_descriptor_heap` migration plan — that
extension is ~1% on Android and is not the target anymore.

Track A as built differs from the plan below in three places:

- **No host-only pools.** Only one CBV_SRV_UAV heap exists (the shader-visible
  one); every heap keeps a CPU `DescriptorRecord` per slot, and a D3D12-style
  copy re-writes the record at the destination instead of `vkCopyDescriptorSets`.
- **Static samplers stay in set 0** (bindings 384..390, immutable samplers),
  no Prism change. The device's list is still the FrameLayout order, hardcoded.
- **DXC heap flags are `<binding> <set>`.** The old `-fvk-bind-sampler-heap 1 0`
  put `SamplerDescriptorHeap` at set 0 / binding 1, not set 1; now `0 1`, plus
  an explicit `-fvk-bind-counter-heap 2 0`.

Open after Track A:

- `VUID-VkWriteDescriptorSet-descriptorType-00328` (storage-buffer offsets must
  be multiples of `minStorageBufferOffsetAlignment`): per-frame placements
  (`GPUEntityStorage::alloc_memory`) and MeshAsset's block starts now align to
  `lcm(stride, alignment)` (`Device::structured_buffer_alignment`). Still open:
  MeshAsset's per-mesh sub-views (`CompiledMeshInfo::vertex_buffer_view` /
  `index_buffer_view` / `meshet_view`) start at `mesh.*_offset * stride` inside
  those blocks, and their offsets are serialized with the asset -- fixing them
  means padding each mesh's segment, i.e. an asset format change + re-import.
  Only the RT hit shaders bind the vertex/index sub-view descriptors.
- `VUID-vkCmdResetQueryPool-renderpass`: GPU-profiler query resets are
  recorded inside an active dynamic-rendering instance. Not descriptor-related.
- Vulkan runs UI-only: the `Spectrum` project now gets `HAL_BACKEND_VULKAN`, so
  `main.cpp`'s 3D scene drawer is excluded, as that guard intended. Rendering
  the scene needs material generation on Vulkan
  (`universal_material::generate_material`).

Two independent tracks:

- **Track A** — binding model: `VK_EXT_mutable_descriptor_type` + descriptor
  indexing, one device-global pipeline layout.
- **Track B** — capabilities: `Raytracing` / `MeshShader` become optional;
  PSOs and passes that need them are skipped.

---

## Why mutable descriptors

A D3D12 CBV/SRV/UAV heap is a flat array: every slot has the same size and can
hold any view type. Classic Vulkan has no such thing — descriptor sizes differ
per type and per GPU, so a `VkDescriptorSet` is a *typed struct* and its
`VkDescriptorSetLayout` is the struct definition. That is why allocation needs a
layout.

Measured on the reference Android device (Xiaomi 15, Adreno 830, driver
512.800.75, gpuinfo report 51382):

| descriptor type | size |
|---|---|
| sampler / sampled image / storage image / uniform buffer | 64 B |
| storage buffer | **192 B** |

`VK_EXT_mutable_descriptor_type` gives one binding whose elements can each be
any type from a declared list — the D3D12 heap semantics — at the cost of every
slot taking the size of the largest listed type. It is what vkd3d-proton uses
to run D3D12 on Vulkan.

Why this and not the alternatives:

| option | Android coverage (gpuinfo) | notes |
|---|---|---|
| `VK_EXT_descriptor_heap` | ~1% | current backend; not viable on Android |
| `VK_EXT_descriptor_buffer` | ~18% | being superseded by descriptor_heap |
| `VK_EXT_mutable_descriptor_type` | ~18% | **chosen** — keeps one index space and `ResourceDescriptorHeap[i]` in HLSL unchanged |
| typed per-type arrays | ~77% (descriptor_indexing) | needs per-type allocators + Prism emitting typed arrays; fallback if mutable ever blocks us |

The mutable coverage figure is not the limiting factor: mesh shaders (~5%) and
RT pipelines (~7.7%) are rarer, and every driver that ships them (Qualcomm
512.800+, Mali r53+, Turnip) also ships mutable descriptors.

Xiaomi 15 also has: update-after-bind for all types, non-uniform indexing for
sampled/storage image, storage buffer **and** UBO, storage-image R/W without
format, BDA, sync2, dynamic rendering, push descriptors, 256 B push constants.
It does **not** have `VK_EXT_mesh_shader` or `VK_KHR_ray_tracing_pipeline`
(only `ray_query`) — hence Track B.

---

## Track A — binding model

### The shape

SIG/Prism uses `DefaultLayout` everywhere, and every slot is a single root
constant holding a bindless index (`HAL/SIG/Layout.ixx`:
`DescriptorConstants(T::ID, 1, ...)`; HLSL reads it as `_hal_push.s{id}` under
`__spirv__`, see `Prism/templates/hlsl/slot.jinja`). So the entire Vulkan
"root signature" is one device-global object:

```
VkPipelineLayout  (one, created at device init, derived from DefaultLayout)
 ├─ set 0: b0 MUTABLE[N] {SAMPLED_IMAGE, STORAGE_IMAGE, STORAGE_BUFFER, UNIFORM_BUFFER}
 │         (+ counter-buffer binding, see open items)
 ├─ set 1: b0 SAMPLER[N]
 ├─ set 2: 7 immutable static samplers (FrameLayout's list)
 └─ push constants: 15 slots × uint = 60 B, VK_SHADER_STAGE_ALL, offset = slot.id * 4
```

`NoneLayout` uses a subset of the same push block, so the same pipeline layout
serves it. All pipelines share one layout → sets bound once stay valid across
`set_pipeline` (D3D12 "SetDescriptorHeaps once" semantics).

### How the objects relate

```
                    HAL::API::Device (built once)
 ┌─────────────────────────────────────────────────────────────────┐
 │ set0_layout, set1_layout            (UPDATE_AFTER_BIND_POOL)     │
 │ set0_host_layout, set1_host_layout  (HOST_ONLY_POOL_BIT_EXT)     │
 │ static_sampler_layout + its one set (set 2)                      │
 │ pipeline_layout = {set0, set1, set2} + 60 B push constants       │
 └──────┬────────────────────────┬─────────────────────┬───────────┘
        │ each heap allocates    │ every PSO uses       │ every bind/push uses
        ▼ one set from these     ▼ the same layout      ▼ the same layout
 DescriptorHeap             PipelineState          CommandList
 ├ ShaderVisible:           vkCreate*Pipelines(    set_descriptor_heaps → remember sets, dirty
 │  pool(UAB) + 1 set;        layout = global)     before draw/dispatch: vkCmdBindDescriptorSets
 │  place() = vkUpdate-     no flags2, no          (only when dirty, per bind point)
 │  DescriptorSets            mapping tables       set_constant → vkCmdPushConstants(slot*4)
 └ CPU-only:                                       reapply_draw_state → rebind + re-push
    pool(HOST_ONLY) + 1 set;
    operator= = vkCopyDescriptorSets
 RootSignature: empty (nothing to create)
```

### A1 — `HAL.Vulkan.Device.ixx / .cpp`

- Require `VK_EXT_mutable_descriptor_type` (+ `mutableDescriptorType` feature)
  and the Vulkan 1.2 descriptor-indexing features: `runtimeDescriptorArray`,
  `descriptorBindingPartiallyBound`, `descriptorBindingVariableDescriptorCount`,
  `descriptorBinding{SampledImage,StorageImage,StorageBuffer,UniformBuffer}UpdateAfterBind`,
  `shader{SampledImage,StorageImage,StorageBuffer,UniformBuffer}ArrayNonUniformIndexing`.
- Delete the `VK_EXT_descriptor_heap` machinery: extension/feature enable,
  `vkGetDeviceProcAddr` trampolines, binding-mapping table, descriptor-size /
  reserved-range queries.
- Create the set layouts, host-only variants, static-sampler set and the global
  pipeline layout; expose getters.
- Clamp heap sizes to `maxDescriptorSetUpdateAfterBind*` — the current
  `65536 * 8` (`HAL.DescriptorHeap.cpp:295`) exceeds a 500k limit.
- Starting point: the classic implementation before commit `bfd6aaf3`
  (`git show bfd6aaf3^:sources/HAL/API/Vulkan/HAL.Vulkan.Device.cpp`) already
  builds a mutable set layout.

### A2 — `HAL.Vulkan.DescriptorHeap.ixx / .cpp`

- Members: `VkDescriptorPool`, `VkDescriptorSet`, `host_only` flag. Remove the
  VMA buffer / mapped pointer / device address / descriptor-size members.
- `place(SRV/UAV/CBV)` → `VkWriteDescriptorSet{ dstBinding = 0,
  dstArrayElement = offset, descriptorType = <concrete type> }`. Keep the
  existing `FirstElement` / `NumElements` / `OffsetInBytes` sub-range handling.
- `operator=` → 1-element `vkCopyDescriptorSets` (mutable descriptors keep their
  type through the copy; works host-only → shader-visible).
- `copy_ranges_to_gpu` stays a no-op: shader-visible heaps are written directly,
  which update-after-bind makes legal for slots in-flight work doesn't read.
- RTV / DSV heaps: unchanged (dynamic rendering, no Vulkan objects).
- Starting point: `git show bfd6aaf3^:sources/HAL/API/Vulkan/HAL.Vulkan.DescriptorHeap.cpp`.

### A3 — `HAL.Vulkan.PipelineState.cpp`

- Remove `VkPipelineCreateFlags2CreateInfo` (descriptor-heap bit) and the
  per-stage `VkShaderDescriptorSetAndBindingMappingInfoEXT` chain.
- `layout = device.get_pipeline_layout()`.

### A4 — `HAL.Vulkan.CommandList.ixx / .cpp`

- `vkCmdBindResourceHeapEXT` / `vkCmdBindSamplerHeapEXT` → dirty flag +
  `vkCmdBindDescriptorSets` for GRAPHICS and COMPUTE (RAY_TRACING later).
- `vkCmdPushDataEXT` → `vkCmdPushConstants(global layout, VK_SHADER_STAGE_ALL,
  slot*4, ...)`. Keep the staging block for `reapply_draw_state` after a
  command-buffer split.
- `set_graphics_signature` / `set_compute_signature` stay no-ops.
- `graphics/compute_set_const_buffer` stays `ASSERT(0)` (root CBV via BDA is
  future work; SIG never needs it).

### A5 — Prism / shaders

- Static samplers: FrameLayout declares **7** (`linearSampler` …
  `linearBorderBlackSampler`). The old classic code hardcoded 5 (s0..s4,
  bindings 384–388) and would drop `vsmShadowSampler` and
  `linearBorderBlackSampler`. Build the immutable-sampler list from
  DefaultLayout's `RootSignatureDesc` (`desc.set_sampler(...)`), not a constant.
- Put them in set 2 so heap layouts don't depend on signature creation order:
  Prism's generated layout header emits `[[vk::binding(i, 2)]]` on the samplers
  under `__spirv__`. (Alternative: set 0 at 384+ with `-fvk-s-shift 384`, only
  if the device knows the sampler list before the first heap is created.)
- DXC flags in `HAL.Vulkan.ShaderReflection.cpp` otherwise unchanged
  (`-fvk-bind-resource-heap 0 0`, `-fvk-bind-sampler-heap 1 0`).

### Track A open items

- **Append/Consume counters**: DXC puts hidden counter buffers at set 0,
  binding 2 (`counter.var.ResourceDescriptorHeap`). Either add a
  `STORAGE_BUFFER[N]` binding for it to set 0, or ban Append/Consume on Vulkan.
  The first descriptor-heap attempt hit exactly this.
- **Memory**: every mutable slot costs the largest listed type (192 B on
  Adreno) → ~96 MiB for 524,288 slots vs 32 MiB at 64 B. Acceptable; keep the
  type list minimal and the heap clamped.
- **TLAS in the heap** (`ACCELERATION_STRUCTURE_KHR` in the mutable list, and
  DXC's `ResourceDescriptorHeap` → `RaytracingAccelerationStructure` path) —
  unverified; only matters once Vulkan RT exists. Fallback: separate binding.

### Outside DefaultLayout (RT / mesh only → gated by Track B)

- **RTX local root signature** (`create_local_signature`, hit-group records) →
  Vulkan shader-record data via `[[vk::shader_record_ext]]`. Deferred with
  Vulkan RT.
- **Per-draw root constants in indirect commands**: `CommandData` and
  `VSMDispatchCommandData` carry slot values (`mesh_cb`, `material_cb`, …) per
  draw — D3D12 `ExecuteIndirect` sets root constants per command, Vulkan cannot
  change push constants inside an indirect draw. The comment at
  `HAL.Vulkan.CommandList.cpp` `execute_indirect` claiming the argument structs
  "carry no per-command root constants" is wrong for these two; per-draw slots
  are most likely dropped and the mesh-task args read from the wrong offset
  (unconfirmed). Only the mesh path uses them today, but the vertex-shader
  fallback will need the same: the shader reads its per-draw slot values from
  the command buffer itself, indexed by `DrawIndex`.

---

## Track B — optional Raytracing / MeshShader

### Data flow

```
 Adapter probe ──► DeviceProperties { rtx, mesh_shader }
                          │
                          ▼
            HAL::Device::supports(Feature)  ◄── debug override: force off
            (the only thing anything else asks)
     ┌──────────────┬─────────┴──────────┬────────────────────────┐
     ▼              ▼                    ▼                        ▼
 Vulkan device   init_pso()          RTX / scene side         FrameGraph pipeline
 creation:       (psos.jinja)        RTX::get() init,         PassNode [Requires=X]
 enable RT/mesh  skip PSOs whose     BLAS/TLAS builds,        → setup condition false
 exts only when  requirement isn't   materials (update_rtx    → consumers: builder.exists()
 present         met                 already gated)           → selectors clamped to non-RT
```

### B1 — capabilities in HAL

- `Device::supports(Feature::Raytracing | Feature::MeshShader)`, next to the
  existing `rtx = !Debug::RunForPix && get_properties().rtx`
  (`HAL.Device.cpp:107`); add `mesh_shader` the same way.
- Force-off override (command-line flag or `Variable<bool>`) — this is how the
  Xiaomi 15 is simulated on a desktop GPU.
- Vulkan device creation: enable mesh (and later RT) extensions only when
  present. `props.mesh_shader` is already probed
  (`HAL.Vulkan.Device.cpp:474`); `props.rtx` stays `false` until Vulkan RT is
  implemented.
- `RenderSystem::select_adapter` already prefers mesh-shader adapters and falls
  back — no change needed.

### B2 — Prism `[Requires = ...]`

- PSOs — inferred:
  - `GraphicsPSO` with `mesh =` / `amplification =` → `MeshShader`
  - `RaytracePSO`, `RaytraceRaygen`, `RaytracePass` → `Raytracing`
  - explicit only: `ComputePSO` using `RayQuery` (Prism can't see HLSL), e.g.
    `RTXShadowReferenceCompute`.
- `psos.jinja` `init_pso`: wrap each `tasks.emplace_back` in
  `if (device.supports(...))`. A skipped PSO stays null → `set_pipeline` needs a
  clear assert naming the PSO.
- PassNode: explicit `[Requires = X]`, ANDed into the generated setup condition
  (reuse the `SetupCondition` codegen in `pass.jinja` / `pass_defaults*.jinja`).
- `Validate.cpp`: allow `Requires` on PSOs and PassNodes.

### B3 — tag passes

| feature | passes / code | gating |
|---|---|---|
| MeshShader | `Scene` (GBuffer/Depth), `VSM_RenderPages`, `Voxelize`, `stencil_renderer`, `DebugView`, `AssetGBuffer`, material preview | these **create** resources (GBuffer, VSM atlas, …): keep setup, skip only the draws, so consumers see cleared targets |
| Raytracing | `ShadowRTX`, `ReflectionRTX(Half)`, `IndirectRTX(Half)`, `RTXCombine`, all `DDGI*`, `RTXShadow`, `RTXColorPass`, `TranslucentRTX` | `[Requires = Raytracing]` — drop the pass |
| Raytracing, outside the graph | `RTX::get()` init, BLAS/TLAS builds (`f_rtx` task, `main.cpp`), `MeshAsset` BLAS | early-out on `supports()` |
| already gated | `universal_material::update_rtx`, VSM `rtx_verify` (`VSM.cpp:1516`) | — |

### B4 — fix consumers

- Clamp selectors to non-RT defaults when RT is off:
  `IndirectGISelectors::indirect_source`, `reflection_source`,
  `VSMSelectors::shadow_source` — otherwise `[Optional = selector]` fields in
  `nrd_sig_test.prism` expect resources no pass produces.
- Run the graph with both features forced off and fix every graph-build failure
  (`need()` without `exists()` on a resource from a dropped pass).
- Tests: skip `Test.RTX` and the mesh/RT parts of `Test.HAL.Rendering` when
  unsupported.

All of Track B is backend-independent and can be built and verified on D3D12
first.

---

## Order

```
A1 Device layouts ─► A2 DescriptorHeap ─► A3 PipelineState ─► A4 CommandList ─► A5 Prism samplers ─► test.exe (Vulkan, desktop)
                                                                                                          │
B1 caps + override ─► B2 Prism [Requires] ─► B3 tag passes/PSOs ─► B4 fix consumers ──────────────────────┤
          (verify on D3D12 first)                                                                         ▼
                                                        Milestone: desktop Vulkan, mesh + RT forced off
                                                                   = what the Xiaomi 15 will run
```

Expected result at the milestone: every scene draw goes through mesh /
amplification shaders (GBuffer, depth, VSM, voxelization), so with mesh
shaders off **the scene is empty**; sky, UI, post-processing and compute passes
work. The vertex-shader fallback pass is the next step after that.

## Constraints (unchanged)

- No `#ifdef` in HAL backend code (`HAL_BACKEND_VULKAN` only outside HAL).
- `#ifdef __spirv__` allowed in HLSL and Prism-generated HLSL.
- WorkGR stays `[ExcludeVulkan]` (DXC lib_6_8 + `-spirv` bug).
