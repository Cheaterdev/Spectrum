module HAL:Resource;

import vulkan;
import Core;

import :HeapAllocators;
import :FrameManager;
#undef THIS

// Vulkan implementation of the common HAL:Resource partition and the
// HAL::API::Resource methods.  Mirrors the structure of
// D3D12/HAL.D3D12.Resource.cpp (which is excluded from the Vulkan build).
// Phase 0: stubs — no real VkBuffer/VkImage creation yet (Phase 1).

namespace HAL
{
    namespace API
    {
        GPUAddressPtr Resource::get_address() { return address; }

        void* Resource::get_cpu_mapping() { return mapped_data; }

        void Resource::init(Device& device, const ResourceDesc& _desc,
                            const PlacementAddress& /*placement*/, TextureLayout initialLayout)
        {
            auto THIS = static_cast<HAL::Resource*>(this);
            THIS->m_device = static_cast<HAL::Device*>(&device);
            THIS->desc = _desc;

            // Phase 1: vmaCreateBuffer / vmaCreateImage, fill vk_buffer/vk_image,
            // set address via vkGetBufferDeviceAddress, map upload/readback heaps.

            THIS->state_manager.init_subres(device.Subresources(THIS->get_desc()), initialLayout);

            if (THIS->heap_type == HeapType::RESERVED)
                THIS->tiled_manager.init_tilings();
        }

        void Resource::init(const NativeImportHandle& handle, TextureLayout layout, Device& device)
        {
            auto THIS = static_cast<HAL::Resource*>(this);
            THIS->m_device = static_cast<HAL::Device*>(&device);

            import_handle = handle;
            vk_image      = handle.image;

            // Imported (e.g. swapchain) images: build a minimal 2D texture desc.
            // Phase 2 fills real dimensions from the swapchain create info.
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
        // Phase 1: allocate memory via VMA / static GPU data, like the D3D12 path.

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
        // Phase 1: vmaDestroyBuffer / vmaDestroyImage for owned resources.
    }
}
