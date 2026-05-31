module HAL:Device;

import :Debug;
import :Utils;

import vulkan;
import Core;

#undef THIS

// Vulkan native implementation of HAL::Device.
// Mirrors the partition layout of D3D12/HAL.D3D12.Device.cpp: this single TU
// (declared `module HAL:Device`) defines BOTH the HAL::API::Device methods and
// the backend-agnostic-but-native HAL::Device::get_texture_layout / compress.
//
// Phase 0: device/instance creation is stubbed. Phase 1 fills it in.

namespace HAL
{
    // ---- Common HAL::Device methods that are implemented natively ----------

    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource)
    {
        // Phase 1: compute via Vulkan format/extent math (no GetCopyableFootprints
        // equivalent — derive row pitch from the format block size).
        auto& desc = rdesc.as_texture();
        auto info = desc.Format.surface_info({ desc.Dimensions.x, desc.Dimensions.y });
        return {
            info.numBytes, info.numRows, info.rowBytes,
            static_cast<uint>(info.numBytes), 256u, desc.Format
        };
    }

    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource, ivec3 box)
    {
        auto& desc = rdesc.as_texture();
        auto info = desc.Format.surface_info({ (uint)box.x, (uint)box.y });
        uint64 res_stride = Math::AlignUp((uint64)info.rowBytes, 256ull);
        uint64 size = res_stride * info.numRows * box.z;
        return {
            size, info.numRows, static_cast<uint>(res_stride),
            static_cast<uint>(res_stride * info.numRows), 512u, desc.Format
        };
    }

    std::vector<std::byte> Device::compress(std::span<std::byte> source)
    {
        // Phase 1+: route through DirectStorage GDeflate codec (cross-platform)
        // or a CPU deflate.  For now pass through uncompressed.
        std::vector<std::byte> dest;
        dest.assign(source.data(), source.data() + source.size());
        return dest;
    }

    // ---- HAL::API::Device methods ------------------------------------------

    namespace API
    {
        void Device::init(DeviceDesc& desc)
        {
            auto THIS = static_cast<HAL::Device*>(this);
            THIS->adapter = desc.adapter;
            vk_physical = desc.adapter ? desc.adapter->vk_physical : VK_NULL_HANDLE;

            // Phase 1:
            //  - vkCreateInstance (+ VK_LAYER_KHRONOS_validation in debug)
            //  - VK_EXT_debug_utils messenger
            //  - vkCreateDevice with: VK_KHR_swapchain, dynamic_rendering,
            //    synchronization2, buffer_device_address, descriptor_indexing,
            //    timeline_semaphore
            //  - vmaCreateAllocator
            //  - fill DeviceProperties (mesh_shader, full_bindless, rtx, ...)
        }

        Device::~Device()
        {
            if (vma_allocator) vmaDestroyAllocator(vma_allocator);
            if (vk_device)     vkDestroyDevice(vk_device, nullptr);
            if (vk_debug_messenger)
            {
                auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
                if (fn) fn(vk_instance, vk_debug_messenger, nullptr);
            }
            if (vk_instance)   vkDestroyInstance(vk_instance, nullptr);
        }

        void Device::process_result(VkResult result, std::string_view line) const
        {
            if (result != VK_SUCCESS)
                Log::get().crash_error(static_cast<int>(result), line);
        }

        uint Device::get_descriptor_size(DescriptorHeapType) const { return 0; }

        VkDevice Device::get_native_device() const { return vk_device; }

        VkResult Device::get_device_removed_reason() const { return VK_SUCCESS; }

        uint Device::Subresources(const ResourceDesc& desc) const
        {
            if (desc.is_buffer()) return 1;
            auto t = desc.as_texture();
            return t.MipLevels * t.ArraySize;
        }

        size_t Device::get_vram() { return 0; }

        ResourceAllocationInfo Device::get_alloc_info(const ResourceDesc& desc)
        {
            // Phase 1: vmaGetXxxMemoryRequirements / vkGetImageMemoryRequirements.
            ResourceAllocationInfo result{};
            result.size      = desc.is_buffer() ? desc.as_buffer().SizeInBytes : 0;
            result.alignment = 256;
            result.flags     = desc.is_buffer() ? HeapFlags::BUFFERS_ONLY : HeapFlags::TEXTURES_ONLY;
            return result;
        }

        RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescBottomInputs&) { return {}; }
        RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescTopInputs&)    { return {}; }
    }
}
