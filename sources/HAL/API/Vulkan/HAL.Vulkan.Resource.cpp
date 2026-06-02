module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
module HAL:Resource;

import Core;

import :Utils;          // to_native(Format)
import :API.Resource;   // API::Resource, PlacementAddress, NativeImportHandle; pulls in :Types, :API.Device
import :API.Heap;       // API::Heap::cpu_address, get_address()
import :Heap;           // HAL::Heap::get_type()
import :Device;         // HAL::Device
import :HeapAllocators;
import :FrameManager;

namespace HAL
{
    namespace API
    {
        GPUAddressPtr Resource::get_address() { return address; }

        void* Resource::get_cpu_mapping() { return mapped_data; }

        void Resource::init(Device& device, const ResourceDesc& _desc,
                            const PlacementAddress& placement, TextureLayout initialLayout)
        {
            auto THIS = static_cast<HAL::Resource*>(this);
            THIS->m_device = static_cast<HAL::Device*>(&device);
            THIS->desc = _desc;

            // Mirror D3D12: set heap_type from the placement heap so that
            // Buffer::init() (HAL.Resource.Buffer.cpp) picks it up correctly.
            if (placement.heap)
                THIS->heap_type = placement.heap->get_type();
            else
                THIS->heap_type = HeapType::RESERVED;

            THIS->state_manager.init_subres(device.Subresources(THIS->get_desc()), initialLayout);

            if (THIS->heap_type == HeapType::RESERVED)
            {
                THIS->tiled_manager.init_tilings();
                return;
            }

            // Placed resources (e.g. the buffer wrapper inside a Heap): the
            // heap owns the VMA memory.  Derive CPU/GPU addresses from the heap
            // rather than creating a separate VMA allocation.
            if (placement.heap)
            {
                auto* api_heap = static_cast<API::Heap*>(placement.heap);
                if (api_heap->cpu_address)
                    mapped_data = api_heap->cpu_address + placement.offset;
                address = api_heap->get_address() + placement.offset;
                return;
            }

            VmaAllocator allocator = device.get_vma_allocator();
            if (!allocator) return;

            VmaAllocationCreateInfo vma_ci{};

            if (_desc.is_buffer())
            {
                auto& buf = _desc.as_buffer();

                VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                bci.size  = buf.SizeInBytes;
                bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                          | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                          | VK_BUFFER_USAGE_INDEX_BUFFER_BIT   | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
                          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                          | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

                if (THIS->heap_type == HeapType::UPLOAD || THIS->heap_type == HeapType::READBACK)
                {
                    vma_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                 | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                    vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
                }
                else
                {
                    vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                }

                VmaAllocationInfo alloc_info{};
                vmaCreateBuffer(allocator, &bci, &vma_ci, &vk_buffer, &vma_alloc, &alloc_info);

                if (THIS->heap_type == HeapType::UPLOAD || THIS->heap_type == HeapType::READBACK)
                    mapped_data = alloc_info.pMappedData;

                if (vk_buffer != VK_NULL_HANDLE)
                {
                    VkBufferDeviceAddressInfo dai{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
                    dai.buffer = vk_buffer;
                    address = vkGetBufferDeviceAddress(device.get_native_device(), &dai);
                }
            }
            else if (_desc.is_texture())
            {
                auto& tex = _desc.as_texture();

                VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
                ici.imageType   = tex.is3D() ? VK_IMAGE_TYPE_3D :
                                  tex.is1D() ? VK_IMAGE_TYPE_1D : VK_IMAGE_TYPE_2D;
                ici.format      = to_native(tex.Format);
                ici.extent      = { tex.Dimensions.x,
                                    tex.is1D() ? 1u : tex.Dimensions.y,
                                    tex.is3D() ? tex.Dimensions.z : 1u };
                ici.mipLevels   = tex.MipLevels;
                ici.arrayLayers = tex.ArraySize;
                ici.samples     = VK_SAMPLE_COUNT_1_BIT;
                ici.tiling      = VK_IMAGE_TILING_OPTIMAL;
                ici.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

                if (check(_desc.Flags & ResFlags::RenderTarget))
                    ici.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                if (check(_desc.Flags & ResFlags::DepthStencil))
                    ici.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

                vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

                vmaCreateImage(allocator, &ici, &vma_ci, &vk_image, &vma_alloc, nullptr);
            }
        }

        void Resource::init(const NativeImportHandle& handle, TextureLayout layout, Device& device)
        {
            auto THIS = static_cast<HAL::Resource*>(this);
            THIS->m_device = static_cast<HAL::Device*>(&device);

            import_handle   = handle;
            vk_image        = handle.image;
            imported_extent = handle.extent;

            // Build a proper 2D TextureDesc so as_texture() works in common code.
            // Mirrors D3D12's Resource::init(NativeImportHandle) which calls extract().
            auto fmt = from_native(handle.format);
            THIS->desc = ResourceDesc::Tex2D(
                fmt,
                uint2(handle.extent.width, handle.extent.height),
                1,   // ArraySize
                1    // MipLevels
            );
            THIS->desc.Flags |= ResFlags::RenderTarget;
            if (layout == TextureLayout::PRESENT)
                THIS->desc.Flags |= ResFlags::Swapchain;

            THIS->state_manager.init_subres(device.Subresources(THIS->get_desc()), layout);
        }
    }

    void Resource::_init(Device& device, const ResourceDesc& desc, HeapType heap_type,
                         TextureLayout initialLayout, vec4 /*clear_value*/)
    {
        m_device = &device;
        this->heap_type = heap_type;
        alloc_info = device.get_alloc_info(desc);

        PlacementAddress address = {};
        init(device, desc, address, initialLayout);
    }

    Resource::Resource(Device& device, const ResourceDesc& desc, HeapType heap_type,
                       TextureLayout initialLayout, vec4 clear_value)
        : state_manager(this), tiled_manager(this)
    {
        _init(device, desc, heap_type, initialLayout, clear_value);
    }

    Resource::Resource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own)
        : state_manager(this), tiled_manager(this)
    {
        m_device = &device;
        PlacementAddress address = { handle.get_heap().get(), handle.get_offset() };
        init(device, desc, address, TextureLayout::UNDEFINED);
        if (own)
            alloc_handle = handle;
    }

    Resource::Resource(Device& device, const ResourceDesc& desc, PlacementAddress address)
        : state_manager(this), tiled_manager(this)
    {
        m_device = &device;
        init(device, desc, address, TextureLayout::UNDEFINED);
    }

    Resource::Resource(Device& device, const API::NativeImportHandle& handle, TextureLayout initialLayout)
        : state_manager(this), tiled_manager(this)
    {
        m_device = &device;
        init(handle, initialLayout, device);
    }

    void Resource::set_name(std::string name)
    {
        if (!this->name.empty() && name.empty()) return;
        this->name = name;
        // Phase 1: vkSetDebugUtilsObjectNameEXT on vk_image / vk_buffer.
    }

    Resource::~Resource()
    {
        alloc_handle.Free();

        if (vma_alloc)
        {
            auto& api_dev = static_cast<API::Device&>(*m_device);
            if (vk_buffer != VK_NULL_HANDLE)
                vmaDestroyBuffer(api_dev.get_vma_allocator(), vk_buffer, vma_alloc);
            else if (vk_image != VK_NULL_HANDLE && !import_handle.image)
                vmaDestroyImage(api_dev.get_vma_allocator(), vk_image, vma_alloc);
            vma_alloc = VK_NULL_HANDLE;
        }
    }

}
