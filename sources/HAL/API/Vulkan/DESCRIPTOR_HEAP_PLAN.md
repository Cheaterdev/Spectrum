# VK_EXT_descriptor_heap Migration Plan

Goal: make the Vulkan HAL backend match D3D12 1:1 in terms of Descriptors,
RootSignatures, and binding features by replacing the classic descriptor model
(VkDescriptorPool / VkDescriptorSet / VkDescriptorSetLayout / VkPipelineLayout)
with **VK_EXT_descriptor_heap**.

Status: **planned** — a previous attempt was reverted; its failure causes are
analyzed at the bottom and folded into this plan.

---

## Why VK_EXT_descriptor_heap

The extension was designed to match D3D12's descriptor model:

| D3D12 concept | Classic Vulkan (current) | VK_EXT_descriptor_heap |
|---|---|---|
| Descriptor heap = flat memory array | VkDescriptorPool + VkDescriptorSet (opaque) | **VkBuffer, host-mapped, device address** |
| CPU handle = ptr + index × increment | impossible; must vkUpdateDescriptorSets | mapped_ptr + index × `resourceDescriptorSize` |
| GPU handle / bindless index | dstArrayElement games on binding 0 | byte offset / index into the heap buffer |
| `CopyDescriptors` = memcpy | vkCopyDescriptorSets (slow, constrained) | **plain memcpy** |
| CPU-only staging heaps | second descriptor set + copy restrictions | plain host allocation, memcpy to GPU heap |
| `SetDescriptorHeaps` (2 heaps, once) | vkCmdBindDescriptorSets per-layout, per-bind-point | `vkCmdBindResourceHeapEXT` / `vkCmdBindSamplerHeapEXT` |
| Root signature = small blob, no layout objects | VkDescriptorSetLayout + VkPipelineLayout per sig | **no objects at all** — set/binding→heap mapping table at pipeline creation |
| `SetRoot32BitConstant` (no layout arg) | vkCmdPushConstants (needs layout) | `vkCmdPushDataEXT` (no layout arg) |
| Root CBV/SRV/UAV = raw GPU VA | not implementable → `ASSERT(0)` today | push a 64-bit `VkDeviceAddress` via PushData |
| Static samplers in root signature | device-global "inline sampler" set hack | embedded-sampler binding mappings |

---

## File-by-file changes

### HAL.Vulkan.Device (.ixx / .cpp)

- Require `VK_EXT_descriptor_heap` **and `VK_KHR_maintenance5`**.
  - descriptor_heap lists maintenance5 as a required dependency
    (VUID-vkCreateDevice-ppEnabledExtensionNames-01387).
  - maintenance5 also provides `VkPipelineCreateFlags2CreateInfo` and allows
    `layout = VK_NULL_HANDLE` at pipeline creation.
  - Enable BOTH feature structs (`VkPhysicalDeviceMaintenance5FeaturesKHR`,
    `VkPhysicalDeviceDescriptorHeapFeaturesEXT`) in the device `pNext` chain.
  - Hard required-extension check with `crash_error` if unsupported
    (AMD iGPU lacks it; NVIDIA 580+ supports it).
- Load extension entry points via `vkGetDeviceProcAddr` trampolines
  (guarded by `VK_ONLY_EXPORTED_PROTOTYPES` in the header — NOT exported by
  vulkan-1.lib): `vkWriteResourceDescriptorsEXT`,
  `vkWriteSamplerDescriptorsEXT`, `vkCmdBindResourceHeapEXT`,
  `vkCmdBindSamplerHeapEXT`, `vkCmdPushDataEXT`.
  Pattern: anonymous-namespace `PFN_*` pointers + real `VKAPI_ATTR` function
  definitions that delegate; load once after `vkCreateDevice`; log per-function
  if null.
- Query `VkPhysicalDeviceDescriptorHeapPropertiesEXT` →
  `resourceDescriptorSize` / `samplerDescriptorSize` become the D3D12
  "handle increment size" (exposed through the existing `handle_size`).
- Build the **binding-mapping table once** (device-lifetime storage — stable
  pointers, pipelines reference it at creation):
  - set 0, binding 0        → resource heap (`-fvk-bind-resource-heap 0 0`)
  - set 0, binding 2        → counter buffer (`counter.var.ResourceDescriptorHeap`,
    DXC puts AppendStructuredBuffer hidden counters here — **was missed in the
    first attempt**, VUID-11312 fired for it)
  - set 0, bindings 384–388 → embedded static samplers s0..s4 (`-fvk-s-shift 384`)
  - set 1, binding 0        → sampler heap (`-fvk-bind-sampler-heap 1 0`)
- Delete: `cbv_srv_uav_layout`, `sampler_layout`, inline-sampler descriptor
  writes, `VK_EXT_mutable_descriptor_type` machinery and extension.

### HAL.Vulkan.DescriptorHeap (.ixx / .cpp)

- Replace pool/set with a **VMA host-visible, device-addressable VkBuffer**
  (AUTO + HOST_ACCESS_RANDOM + MAPPED, persistently mapped) sized
  `Count × descriptorSize` plus the driver-reported reserved range.
- `Descriptor::place(...)` → fill `VkResourceDescriptorInfoEXT` and call
  `vkWriteResourceDescriptorsEXT` writing into
  `mapped_ptr + offset × descriptorSize`. No UPDATE_AFTER_BIND, no pool limits.
  - Signature note: `vkWriteResourceDescriptorsEXT(VkDevice, uint32_t count,
    const VkResourceDescriptorInfoEXT*, const VkHostAddressRangeEXT*)` —
    destination is the *mutable* `VkHostAddressRangeEXT`.
- `Descriptor::operator=` (copy) → **memcpy** between heaps' mapped memory —
  exact D3D12 `CopyDescriptors` semantics, including CPU-staging → GPU-visible
  copies that are impossible with vkCopyDescriptorSets today.
- Non-shader-visible heaps (D3D12 `FLAG_NONE`): plain host memory, no VkBuffer.
- RTV/DSV heaps: unchanged (no Vulkan objects; dynamic rendering).
- Resource side: keep `VkImageViewCreateInfo` alongside `VkImageView` so
  descriptor writes can be described (the EXT wants create-info, not views,
  for some paths).

### HAL.Vulkan.RootSignature (.ixx / .cpp)

- Becomes a near-empty class: no VkPipelineLayout, no VkDescriptorSetLayout,
  no destructor logic. Keeps only the CPU-side interpretation of
  `RootSignatureDesc`: which push-data offsets hold 32-bit constants vs.
  root-descriptor addresses — same role the root signature plays for the
  D3D12 command list.
- Static samplers from `RootSignatureDesc::sampler_map` → per-signature
  embedded-sampler mapping entries (replacing the device-global 5-sampler
  hack; matches D3D12 semantics where static samplers belong to the root
  signature).

### HAL.Vulkan.PipelineState (.cpp)

- `VkPipelineCreateFlags2CreateInfo{ .flags =
  VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT }` on the pipeline pNext.
- `VkShaderDescriptorSetAndBindingMappingInfoEXT` chained on **every
  `VkPipelineShaderStageCreateInfo::pNext`** — NOT the pipeline-level chain.
  (First attempt's main bug: VUID-VkGraphicsPipelineCreateInfo-flags-11312 /
  VkComputePipelineCreateInfo-flags-11312 fired for VS/PS/CS/task/mesh stages;
  the struct is also not in the pipeline pNext allowlist, which produced the
  "unexpected VkStructureType" warnings.)
- `layout = VK_NULL_HANDLE` (allowed with maintenance5 + the heap bit).
- If a stage struct needs a local mutation, copy it — no const_cast on
  device-owned mapping storage.

### HAL.Vulkan.CommandList (.ixx / .cpp)

- `set_descriptor_heaps` → capture heap device address / size / reserved
  range; bind lazily before draw/dispatch via `vkCmdBindResourceHeapEXT` /
  `vkCmdBindSamplerHeapEXT` with `VkBindHeapInfoEXT{ heapRange,
  reservedRangeOffset, reservedRangeSize }`. Heaps bind once and persist —
  matches D3D12; delete `flush_descriptor_sets` and per-dispatch rebinding.
- `graphics/compute_set_constant` → `vkCmdPushDataEXT` with
  `VkPushDataInfoEXT{ offset, data = { address, size } }`. Keep the 128-byte
  staging block so `reapply_draw_state` can re-push after a command-buffer
  split (task-based recorder).
- `graphics/compute_set_const_buffer` (today `ASSERT(0)`) → push the buffer's
  `VkDeviceAddress` as 8 bytes of push data at the root-parameter offset.
  Caveat: HLSL must read it as an address (`vk::RawBufferLoad` or
  VK_KHR_shader_untyped_pointers under `#ifdef __spirv__`) — stage this LAST;
  everything else works without it because the SIG system passes bindless
  indices through root constants.
- `set_graphics_signature` / `set_compute_signature` → no-ops (nothing to bind).
- Remove `current_pipeline_layout`, `cbv_srv_uav_set`, `sampler_set`,
  `descriptor_sets_dirty` state.

### HAL.Vulkan.ShaderReflection.cpp (DXC flags)

- **Unchanged.** Existing `-fvk-b/t/u/s-shift`, `-fvk-bind-resource-heap 0 0`,
  `-fvk-bind-sampler-heap 1 0` produce exactly the set/binding decorations the
  mapping table translates.

---

## Constraints (unchanged from project rules)

- NO `#ifdef` in HAL backend code (`HAL_BACKEND_VULKAN` only outside HAL).
- `#ifdef __spirv__` allowed in HLSL shaders.
- NO topology sync from PSO in `set_pipeline`.
- WorkGR stays `[ExcludeVulkan]` (DXC lib_6_8 + -spirv bug).

## Lessons from the reverted first attempt

1. `VK_KHR_maintenance5` must be explicitly enabled (extension list + feature
   struct) — root cause of the device-creation warning and the
   "VkPipelineCreateFlags2CreateInfo but maintenance5 not enabled" spam.
2. Mapping info goes **per-stage**, not per-pipeline.
3. Counter binding (set 0, binding 2) needs its own mapping entry.
4. Extension functions need `vkGetDeviceProcAddr` trampolines + a hard
   required-extension check.
5. Validation layers (SDK 1.4.350) don't list the mapping struct in the
   pipeline pNext allowlist — chaining per-stage sidesteps that warning too.
6. `VkHostAddressRangeConstEXT` field is `address`, not `pAddress`.
7. Module note: extension types may not resolve through the `vulkan` header
   unit — use a `module;` global fragment with `#include <vulkan/vulkan.h>`
   where needed (Device.ixx precedent).

## Implementation order

1. Device: extensions + features + trampolines + descriptor-heap properties +
   mapping table.
2. DescriptorHeap: buffer-backed heaps, place() via write, operator= via memcpy.
3. PipelineState: flags2 + per-stage mapping chaining, null layout.
4. CommandList: heap binding + PushData constants.
5. RootSignature: strip to CPU-side desc holder + embedded static samplers.
6. Root CBV/SRV/UAV via device address (+ HLSL `__spirv__` read path) — last,
   optional until something needs `set_const_buffer` on Vulkan.

Verify each stage against the test suite (`test.exe`): 214 tests, the 18
GUI/SIG texture-mismatch failures from the first attempt are the regression
signal for descriptor binding correctness.
