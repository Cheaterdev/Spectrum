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
// Descriptor model:
//   Set 0 (CBV_SRV_UAV) — 4 bindings sharing one VkDescriptorSet:
//       binding 0: SAMPLED_IMAGE  (SRV textures, indexed by offset)
//       binding 1: STORAGE_IMAGE  (UAV textures, indexed by offset)
//       binding 2: UNIFORM_BUFFER (CBVs,          indexed by offset)
//       binding 3: STORAGE_BUFFER (SRV/UAV buffers, indexed by offset)
//   Set 1 (SAMPLER) — 1 binding:
//       binding 0: SAMPLER        (indexed by offset)
//   RTV / DSV — no VkDescriptorSet; handled by dynamic rendering.
//
// get_gpu() returns the descriptor slot offset so shaders can use it as a
// bindless array index.  All bindings use UPDATE_AFTER_BIND + PARTIALLY_BOUND.

namespace HAL
{
    // ---- Descriptor (slot within a heap) -----------------------------------

    Descriptor::Descriptor(DescriptorHeap& heap, uint offset) : heap(heap), offset(offset) {}

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
            // get_vk_image_view() returns swapchain view or owned view (Phase 5).
            VkImageView view = api_res.get_vk_image_view();
            if (view == VK_NULL_HANDLE) return;
            img_info.imageView   = view;
            img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            write.pImageInfo     = &img_info;
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            // Buffer SRV — binding 3 (STORAGE_BUFFER)
            buf_info.buffer = api_res.get_vk_buffer();
            buf_info.offset = 0;
            buf_info.range  = VK_WHOLE_SIZE;
            write.dstBinding     = 3;
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
            VkImageView view = api_res.get_vk_image_view();
            if (view == VK_NULL_HANDLE) return;
            img_info.imageView   = view;
            img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            write.dstBinding     = 1; // STORAGE_IMAGE
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            write.pImageInfo     = &img_info;
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            buf_info.buffer = api_res.get_vk_buffer();
            buf_info.offset = 0;
            buf_info.range  = VK_WHOLE_SIZE;
            write.dstBinding     = 3; // STORAGE_BUFFER
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

        VkDescriptorBufferInfo buf_info{};
        buf_info.buffer = api_res.get_vk_buffer();
        buf_info.offset = 0;
        buf_info.range  = VK_WHOLE_SIZE;

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet          = api_heap.get_vk_set();
        write.dstBinding      = 2; // UNIFORM_BUFFER
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

    D3D12_CPU_DESCRIPTOR_HANDLE Descriptor::get_cpu()
    {
        // Return the slot index as the cpu handle value (for resource info tracking).
        return { static_cast<SIZE_T>(offset) };
    }

    D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::get_gpu()
    {
        // Bindless index: the shader accesses heap[offset] in the descriptor array.
        return { static_cast<UINT64>(offset) };
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
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER,        d.Count });
            }
            else
            {
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  d.Count });
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  d.Count });
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, d.Count });
                pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, d.Count });
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

        Descriptor DescriptorHeap::operator[](uint i)
        {
            auto THIS = static_cast<HAL::DescriptorHeap*>(this);
            return Descriptor{ *THIS, i };
        }
    }
}
