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

            // Resolve MipLevels=0 → full mip chain.
            // D3D12 handles this natively inside CreateCommittedResource;
            // Vulkan requires mipLevels >= 1 and we must compute the count
            // ourselves before init_subres or vmaCreateImage sees the descriptor.
            if (THIS->desc.is_texture())
            {
                auto& t = THIS->desc.as_texture();
                if (t.MipLevels == 0)
                {
                    uint max_dim = std::max({ t.Dimensions.x,
                                              t.is1D() ? 1u : t.Dimensions.y,
                                              t.is3D() ? t.Dimensions.z : 1u });
                    t.MipLevels = max_dim > 0u
                        ? static_cast<uint>(std::floor(std::log2(
                              static_cast<float>(max_dim)))) + 1u
                        : 1u;
                }
            }

            // heap_type priority:
            //   1. If placement.heap is set, derive from that heap's type.
            //   2. Otherwise, preserve what _init() already wrote.
            //      _init(HeapType::DEFAULT) calls init() with a null placement
            //      but has pre-set heap_type = DEFAULT; overwriting it to RESERVED
            //      was the root bug — it caused every regular non-placed resource
            //      to take the tiled/reserved early-return path with no VkBuffer/Image.
            if (placement.heap)
                THIS->heap_type = placement.heap->get_type();
            // else: keep heap_type as set by caller (_init always pre-sets it)

            THIS->state_manager.init_subres(device.Subresources(THIS->get_desc()), initialLayout);

            if (THIS->heap_type == HeapType::RESERVED)
            {
                THIS->tiled_manager.init_tilings();
                return;
            }

            // Placed resources — two cases:
            //
            // A) UPLOAD / READBACK heap: the heap owns a persistently-mapped VMA
            //    buffer.  Derive cpu/gpu addresses from it (no separate allocation).
            //
            // B) DEFAULT heap: Vulkan has no D3D12-style placed resource without
            //    VK_KHR_bind_memory2 / explicit memory management.  Fall through
            //    to create a standalone VMA allocation instead.  The heap's memory
            //    region is intentionally ignored — the HeapType drives VMA flags.
            if (placement.heap)
            {
                auto* api_heap = static_cast<API::Heap*>(placement.heap);
                if (api_heap->cpu_address)
                {
                    // Case A: CPU-visible heap (UPLOAD / READBACK).
                    // Share the heap's single VkBuffer; the slice is addressed via
                    // resource_offset (placement.offset) at copy/descriptor time.
                    // Without this, get_vk_buffer() is null for every placed staging
                    // buffer / CBV / vertex-index buffer, so copies and descriptor
                    // writes are silently skipped and the GPU reads zeroes.
                    mapped_data = api_heap->cpu_address + placement.offset;
                    address     = api_heap->get_address() + placement.offset;
                    vk_buffer   = api_heap->get_vk_buffer();
                    return;
                }
                // Case B: DEFAULT heap — fall through to VMA allocation below.
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

                if (vk_image != VK_NULL_HANDLE)
                {
                    // Store pixel dimensions so get_imported_extent() works for all images.
                    imported_extent = { ici.extent.width, ici.extent.height };

                    // Create a full-resource image view.
                    bool is_depth = check(_desc.Flags & ResFlags::DepthStencil);
                    VkImageViewCreateInfo ivci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
                    ivci.image    = vk_image;
                    ivci.viewType = tex.is3D()            ? VK_IMAGE_VIEW_TYPE_3D
                                  : tex.is1D()            ? VK_IMAGE_VIEW_TYPE_1D
                                  : tex.ArraySize > 1     ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                                          : VK_IMAGE_VIEW_TYPE_2D;
                    ivci.format   = ici.format;
                    ivci.subresourceRange.aspectMask     = is_depth
                                                         ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                         : VK_IMAGE_ASPECT_COLOR_BIT;
                    ivci.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
                    ivci.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
                    vkCreateImageView(device.get_native_device(), &ivci, nullptr, &vk_image_view);
                }
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
            VkDevice vk_dev = api_dev.get_native_device();

            // Destroy the owned image view before destroying the image itself.
            if (vk_image_view != VK_NULL_HANDLE && !import_handle.image)
            {
                vkDestroyImageView(vk_dev, vk_image_view, nullptr);
                vk_image_view = VK_NULL_HANDLE;
            }

            if (vk_buffer != VK_NULL_HANDLE)
                vmaDestroyBuffer(api_dev.get_vma_allocator(), vk_buffer, vma_alloc);
            else if (vk_image != VK_NULL_HANDLE && !import_handle.image)
                vmaDestroyImage(api_dev.get_vma_allocator(), vk_image, vma_alloc);
            vma_alloc = VK_NULL_HANDLE;
        }
    }

}
