module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:DescriptorHeap;

import :Debug;
import :Resource;
import :Resource.Buffer;
import :API.Device;   // get_native_device(), get_cbv_srv_uav_layout(), etc.

import vulkan;
import Core;

// Vulkan Phase 4 — real descriptor heap implementation.
//
// Descriptor model (binding numbers match DXC -fvk-b/t/u-shift flags):
//   Set 0 (CBV_SRV_UAV) — 4 bindings sharing one VkDescriptorSet:
//       binding   0: UNIFORM_BUFFER (CBVs,          b-shift = 0)
//       binding 128: SAMPLED_IMAGE  (SRV textures,  t-shift = 128)
//       binding 256: STORAGE_IMAGE  (UAV textures,  u-shift = 256)
//       binding 384: STORAGE_BUFFER (SRV/UAV bufs,  u-shift = 256 buffers)
//   Set 1 (SAMPLER) — 1 binding:
//       binding 0: SAMPLER          (-fvk-bind-sampler-heap 1 0)
//   RTV / DSV — no VkDescriptorSet; handled by dynamic rendering.
//
// get_gpu() returns the raw descriptor slot offset as the bindless array index.
// The offset is used directly as dstArrayElement; each binding has HEAP_MAX
// elements so any flat heap position fits regardless of type.
// All bindings use UPDATE_AFTER_BIND + PARTIALLY_BOUND.

namespace HAL
{
    // ---- Descriptor (slot within a heap) -----------------------------------

    Descriptor::Descriptor(DescriptorHeap& heap, uint offset) : heap(heap), offset(offset)
    {
        // Vulkan has no CPU/GPU descriptor handles; the bindless slot index is
        // carried in the stub handle value so the shared HAL::Handle interface
        // (get_cpu/get_gpu) keeps working unchanged.
        cpu_handle = { static_cast<SIZE_T>(offset) };
        gpu_handle = { static_cast<UINT64>(offset) };
    }

    void Descriptor::place(const Views::ShaderResource& v)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!api_heap.get_vk_set() || !v.Resource) return;

        auto& api_res = static_cast<const API::Resource&>(*v.Resource);

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet     = api_heap.get_vk_set();
        write.dstBinding = 0; // SAMPLED_IMAGE by default
        write.dstArrayElement = offset;
        write.descriptorCount = 1;

        VkDescriptorImageInfo  img_info{};
        VkDescriptorBufferInfo buf_info{};

        if (api_res.get_vk_image() != VK_NULL_HANDLE)
        {
            // SRV texture → binding 0 (MUTABLE → SAMPLED_IMAGE)
            VkImageView view = api_res.get_vk_image_view();
            if (view == VK_NULL_HANDLE) return;
            img_info.imageView   = view;
            img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            write.dstBinding     = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            write.pImageInfo     = &img_info;
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            // Buffer SRV → binding 0 (MUTABLE → STORAGE_BUFFER).
            // StructuredBuffers are sub-ranges (FirstElement..NumElements) of a shared
            // buffer; honour the offset/size or the shader reads the wrong elements.
            buf_info.buffer = api_res.get_vk_buffer();
            buf_info.offset = 0;
            buf_info.range  = VK_WHOLE_SIZE;
            if (auto* b = std::get_if<Views::ShaderResource::Buffer>(&v.View))
            {
                const uint64_t stride = b->StructureByteStride ? b->StructureByteStride : 1u;
                buf_info.offset = b->FirstElement * stride;
                if (b->NumElements)
                    buf_info.range = static_cast<VkDeviceSize>(b->NumElements) * stride;
            }
            write.dstBinding     = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo    = &buf_info;
        }
        else return;

        vkUpdateDescriptorSets(api_heap.device.get_native_device(), 1, &write, 0, nullptr);
    }

    void Descriptor::place(const Views::UnorderedAccess& v)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!api_heap.get_vk_set() || !v.Resource) return;

        auto& api_res = static_cast<const API::Resource&>(*v.Resource);

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet          = api_heap.get_vk_set();
        write.dstArrayElement = offset;
        write.descriptorCount = 1;

        VkDescriptorImageInfo  img_info{};
        VkDescriptorBufferInfo buf_info{};

        if (api_res.get_vk_image() != VK_NULL_HANDLE)
        {
            // UAV texture → binding 0 (MUTABLE → STORAGE_IMAGE)
            VkImageView view = api_res.get_vk_image_view();
            if (view == VK_NULL_HANDLE) return;
            img_info.imageView   = view;
            img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            write.dstBinding     = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            write.pImageInfo     = &img_info;
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            // UAV buffer → binding 0 (MUTABLE → STORAGE_BUFFER).  Honour the
            // FirstElement/NumElements sub-range like the SRV path.
            buf_info.buffer = api_res.get_vk_buffer();
            buf_info.offset = 0;
            buf_info.range  = VK_WHOLE_SIZE;
            if (auto* b = std::get_if<Views::UnorderedAccess::Buffer>(&v.View))
            {
                const uint64_t stride = b->StructureByteStride ? b->StructureByteStride : 1u;
                buf_info.offset = static_cast<VkDeviceSize>(b->FirstElement) * stride;
                if (b->NumElements)
                    buf_info.range = static_cast<VkDeviceSize>(b->NumElements) * stride;
            }
            write.dstBinding     = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo    = &buf_info;
        }
        else return;

        vkUpdateDescriptorSets(api_heap.device.get_native_device(), 1, &write, 0, nullptr);
    }

    void Descriptor::place(const Views::ConstantBuffer& v)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!api_heap.get_vk_set() || !v.Resource) return;

        auto& api_res = static_cast<const API::Resource&>(*v.Resource);
        if (api_res.get_vk_buffer() == VK_NULL_HANDLE) return;

        // CRITICAL: constant buffers are sub-allocated inside larger shared buffers
        // (DataHolder::compile → place_data with resource_offset).  The view carries
        // OffsetInBytes/SizeInBytes; ignoring them (offset=0, range=WHOLE) points the
        // descriptor at the START of the shared buffer instead of this CB's sub-region,
        // so the shader reads the wrong bindless indices → garbage vertices → black UI.
        VkDescriptorBufferInfo buf_info{};
        buf_info.buffer = api_res.get_vk_buffer();
        buf_info.offset = v.OffsetInBytes;
        buf_info.range  = v.SizeInBytes > 0 ? v.SizeInBytes : VK_WHOLE_SIZE;

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet          = api_heap.get_vk_set();
        write.dstBinding      = 0; // MUTABLE → UNIFORM_BUFFER (CBV, b-shift = 0)
        write.dstArrayElement = offset;
        write.descriptorCount = 1;
        write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pBufferInfo     = &buf_info;

        vkUpdateDescriptorSets(api_heap.device.get_native_device(), 1, &write, 0, nullptr);
    }

    void Descriptor::place(const Views::RenderTarget&)    {} // handled by dynamic rendering
    void Descriptor::place(const Views::DepthStencil&)    {} // handled by dynamic rendering

    void Descriptor::operator=(const Descriptor& r)
    {
        // Copy-assign a slot: re-write whatever view is stored there.
        // Phase 4: walk the ResourceInfo and call the appropriate place().
        (void)r;
    }

    uint DescriptorHeap::get_size() { return desc.Count; }

    namespace API
    {
        DescriptorHeap::DescriptorHeap(Device& dev, const DescriptorHeapDesc& d)
            : device(dev), desc(d)
        {
            handle_size = 0;

            // RTV / DSV heaps have no Vulkan descriptor objects.
            if (d.HeapType == DescriptorHeapType::RTV ||
                d.HeapType == DescriptorHeapType::DSV)
                return;

            VkDevice vk_dev = dev.get_native_device();
            if (vk_dev == VK_NULL_HANDLE) return;

            // ---- Pool -------------------------------------------------------
            bool is_sampler = (d.HeapType == DescriptorHeapType::SAMPLER);

            std::vector<VkDescriptorPoolSize> pool_sizes;
            if (is_sampler)
            {
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER,         d.Count });
            }
            else
            {
                // Binding 0 is MUTABLE_EXT — one pool entry covers all resource types.
                // Bindings 384-388: 5 inline static samplers (s0..s4), one per binding.
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_MUTABLE_EXT, d.Count });
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER,     5u       });
            }

            VkDescriptorPoolCreateInfo pool_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
            pool_ci.maxSets       = 1;
            pool_ci.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
            pool_ci.pPoolSizes    = pool_sizes.data();
            pool_ci.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

            if (vkCreateDescriptorPool(vk_dev, &pool_ci, nullptr, &vk_pool) != VK_SUCCESS)
                return;

            // ---- Set --------------------------------------------------------
            VkDescriptorSetLayout layout = is_sampler
                ? dev.get_sampler_layout()
                : dev.get_cbv_srv_uav_layout();

            if (layout == VK_NULL_HANDLE) return;

            // Variable descriptor count: use desc.Count for the last binding.
            uint32_t var_count = d.Count;
            VkDescriptorSetVariableDescriptorCountAllocateInfo var_info{
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
            var_info.descriptorSetCount = 1;
            var_info.pDescriptorCounts  = &var_count;

            VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
            alloc_info.descriptorPool     = vk_pool;
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts        = &layout;
            // Only attach variable-count info if the layout actually uses it.
            // The global layout uses PARTIALLY_BOUND but not VARIABLE_DESCRIPTOR_COUNT
            // on the last binding (we use a fixed max count), so skip var_info here.
            // alloc_info.pNext = &var_info;   // reserved for per-heap variable layouts

            vkAllocateDescriptorSets(vk_dev, &alloc_info, &vk_set);
        }

        DescriptorHeap::~DescriptorHeap()
        {
            VkDevice vk_dev = device.get_native_device();
            if (vk_pool != VK_NULL_HANDLE && vk_dev != VK_NULL_HANDLE)
                vkDestroyDescriptorPool(vk_dev, vk_pool, nullptr);
        }

        HAL::Descriptor DescriptorHeap::operator[](uint i)
        {
            auto THIS = static_cast<HAL::DescriptorHeap*>(this);
            return HAL::Descriptor{ *THIS, i };
        }
    }
}
