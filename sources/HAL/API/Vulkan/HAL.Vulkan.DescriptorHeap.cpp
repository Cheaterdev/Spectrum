module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
module HAL:DescriptorHeap;

import :Debug;
import :Resource;
import :Resource.Buffer;
import :API.Device;   // get_native_device(), get_vma_allocator(), descriptor sizes

import vulkan;
import Core;

// VK_EXT_descriptor_heap — D3D12-identical descriptor model.
//
// A heap is a host-visible, device-addressable VkBuffer of Count descriptors
// at a uniform stride (descriptor_size).  A descriptor "handle" is simply
// base + slot*stride, exactly like a D3D12 CPU descriptor handle:
//   - place(view)   → vkWriteResourceDescriptorsEXT into mapped[slot*stride]
//   - operator=     → memcpy between mapped heaps (D3D12 CopyDescriptors)
//   - RTV / DSV     → no descriptor object; handled by dynamic rendering
//
// DXC SPIR-V indexes ResourceDescriptorHeap[i] as element i of set 0 binding 0;
// the Device's mapping table converts that to base + i*stride.  So the slot
// index IS the bindless array index the shader uses.

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

    // Write one resource descriptor into the heap's mapped memory at `slot`.
    static void write_descriptor(API::DescriptorHeap& heap, uint slot,
                                 VkDescriptorType type,
                                 const VkResourceDescriptorDataEXT& data)
    {
        uint8_t* dst_ptr = heap.get_mapped();
        if (!dst_ptr) return;

        VkResourceDescriptorInfoEXT info{ VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT };
        info.type = type;
        info.data = data;

        VkHostAddressRangeEXT dst{};
        dst.address = dst_ptr + slot * heap.get_descriptor_size();
        dst.size    = static_cast<size_t>(heap.get_descriptor_size());

        vkWriteResourceDescriptorsEXT(heap.device.get_native_device(), 1, &info, &dst);
    }

    void Descriptor::place(const Views::ShaderResource& v, bool /*skip_gpu_write*/)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!api_heap.get_mapped() || !v.Resource) return;

        auto& api_res = static_cast<const API::Resource&>(*v.Resource);

        if (api_res.get_vk_image() != VK_NULL_HANDLE)
        {
            // SRV texture → SAMPLED_IMAGE.  GENERAL layout matches
            // to_native(SHADER_RESOURCE) so SRV/UAV slots for the same image agree.
            VkImageViewCreateInfo view_ci = api_res.get_view_ci();
            if (view_ci.image == VK_NULL_HANDLE) return;

            VkImageDescriptorInfoEXT img{ VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT };
            img.pView  = &view_ci;
            img.layout = VK_IMAGE_LAYOUT_GENERAL;

            VkResourceDescriptorDataEXT data{};
            data.pImage = &img;
            write_descriptor(api_heap, offset, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, data);
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            // Buffer SRV → STORAGE_BUFFER via device address.  StructuredBuffers are
            // sub-ranges (FirstElement..NumElements); honour them or the shader reads
            // the wrong elements.
            VkDeviceAddress base = static_cast<VkDeviceAddress>(
                const_cast<API::Resource&>(api_res).get_address());
            VkDeviceSize    off  = 0;
            VkDeviceSize    range = VK_WHOLE_SIZE;
            if (auto* b = std::get_if<Views::ShaderResource::Buffer>(&v.View))
            {
                const uint64_t stride = b->StructureByteStride ? b->StructureByteStride : 1u;
                off = b->FirstElement * stride;
                range = b->NumElements ? static_cast<VkDeviceSize>(b->NumElements) * stride
                                       : VK_WHOLE_SIZE;
            }
            else ASSERT(0);

            VkDeviceAddressRangeEXT ar{};
            ar.address = base + off;
            ar.size    = range;

            VkResourceDescriptorDataEXT data{};
            data.pAddressRange = &ar;
            write_descriptor(api_heap, offset, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data);
        }
    }

    void Descriptor::place(const Views::UnorderedAccess& v, bool /*skip_gpu_write*/)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!api_heap.get_mapped() || !v.Resource) return;

        auto& api_res = static_cast<const API::Resource&>(*v.Resource);

        if (api_res.get_vk_image() != VK_NULL_HANDLE)
        {
            // UAV texture → STORAGE_IMAGE.  Vulkan requires a single-mip view.
            uint32_t mip_slice = 0, array_slice = 0;
            std::visit(overloaded{
                [&](const Views::UnorderedAccess::Texture2D& t)      { mip_slice = t.MipSlice; },
                [&](const Views::UnorderedAccess::Texture2DArray& t) { mip_slice = t.MipSlice; array_slice = t.FirstArraySlice; },
                [&](const Views::UnorderedAccess::Texture3D& t)      { mip_slice = t.MipSlice; },
                [](auto&&) {}
            }, v.View);

            VkImageViewCreateInfo view_ci = api_res.get_uav_view_ci(mip_slice, array_slice);
            if (view_ci.image == VK_NULL_HANDLE) return;

            VkImageDescriptorInfoEXT img{ VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT };
            img.pView  = &view_ci;
            img.layout = VK_IMAGE_LAYOUT_GENERAL;

            VkResourceDescriptorDataEXT data{};
            data.pImage = &img;
            write_descriptor(api_heap, offset, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, data);
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            VkDeviceAddress base = static_cast<VkDeviceAddress>(
                const_cast<API::Resource&>(api_res).get_address());
            VkDeviceSize    off  = 0;
            VkDeviceSize    range = VK_WHOLE_SIZE;
            if (auto* b = std::get_if<Views::UnorderedAccess::Buffer>(&v.View))
            {
                const uint64_t stride = b->StructureByteStride ? b->StructureByteStride : 1u;
                off = static_cast<VkDeviceSize>(b->FirstElement) * stride;
                range = b->NumElements ? static_cast<VkDeviceSize>(b->NumElements) * stride
                                       : VK_WHOLE_SIZE;
            }

            VkDeviceAddressRangeEXT ar{};
            ar.address = base + off;
            ar.size    = range;

            VkResourceDescriptorDataEXT data{};
            data.pAddressRange = &ar;
            write_descriptor(api_heap, offset, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data);
        }
    }

    void Descriptor::place(const Views::ConstantBuffer& v, bool /*skip_gpu_write*/)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!api_heap.get_mapped() || !v.Resource) return;

        auto& api_res = static_cast<const API::Resource&>(*v.Resource);
        if (api_res.get_vk_buffer() == VK_NULL_HANDLE) return;

        // CBVs are sub-allocated inside larger shared buffers; honour
        // OffsetInBytes/SizeInBytes or the shader reads the wrong sub-region.
        VkDeviceAddress base = static_cast<VkDeviceAddress>(
            const_cast<API::Resource&>(api_res).get_address());

        VkDeviceAddressRangeEXT ar{};
        ar.address = base + v.OffsetInBytes;
        ar.size    = v.SizeInBytes > 0 ? v.SizeInBytes : VK_WHOLE_SIZE;

        VkResourceDescriptorDataEXT data{};
        data.pAddressRange = &ar;
        write_descriptor(api_heap, offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, data);
    }

    void Descriptor::place(const Views::RenderTarget&, bool) {} // handled by dynamic rendering
    void Descriptor::place(const Views::DepthStencil&, bool) {} // handled by dynamic rendering

    void Descriptor::operator=(const Descriptor& r)
    {
        // D3D12 CopyDescriptors == memcpy between mapped heaps.  RTV/DSV heaps have
        // no descriptor memory (dynamic rendering) — the ResourceInfo copy done by
        // HAL::Handle::place() is enough there.
        auto& dst_api = static_cast<API::DescriptorHeap&>(heap);
        auto& src_api = static_cast<API::DescriptorHeap&>(r.heap);

        if (dst_api.desc.HeapType == DescriptorHeapType::RTV ||
            dst_api.desc.HeapType == DescriptorHeapType::DSV)
            return;

        if (!dst_api.get_mapped() || !src_api.get_mapped()) return;

        const VkDeviceSize stride = dst_api.get_descriptor_size();
        std::memcpy(dst_api.get_mapped() + offset   * stride,
                    src_api.get_mapped() + r.offset  * src_api.get_descriptor_size(),
                    static_cast<size_t>(stride));
    }

    uint DescriptorHeap::get_size() { return desc.Count; }

    namespace API
    {
        DescriptorHeap::DescriptorHeap(Device& dev, const DescriptorHeapDesc& d)
            : device(dev), desc(d)
        {
            handle_size = 0;

            // RTV / DSV heaps have no descriptor memory (dynamic rendering).
            if (d.HeapType == DescriptorHeapType::RTV ||
                d.HeapType == DescriptorHeapType::DSV)
                return;

            VkDevice     vk_dev = dev.get_native_device();
            VmaAllocator vma    = dev.get_vma_allocator();
            if (vk_dev == VK_NULL_HANDLE || vma == VK_NULL_HANDLE) return;

            const bool is_sampler = (d.HeapType == DescriptorHeapType::SAMPLER);
            descriptor_size = is_sampler ? dev.get_sampler_descriptor_size()
                                         : dev.get_resource_descriptor_size();
            reserved_size   = is_sampler ? dev.get_sampler_reserved_range()
                                         : dev.get_resource_reserved_range();
            handle_size     = static_cast<uint>(descriptor_size);
            if (descriptor_size == 0) return;

            main_size       = descriptor_size * d.Count;
            reserved_offset = main_size;                 // reserved (embedded) range at the end
            const VkDeviceSize total = main_size + reserved_size;

            const VkDeviceSize align = is_sampler ? dev.get_sampler_heap_alignment()
                                                  : dev.get_resource_heap_alignment();

            VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bci.size  = total;
            bci.usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT
                      | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

            VmaAllocationCreateInfo aci{};
            aci.usage = VMA_MEMORY_USAGE_AUTO;
            aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
                      | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo ai{};
            VkResult r = vmaCreateBufferWithAlignment(
                vma, &bci, &aci, align ? align : 1, &vk_heap_buffer, &vma_alloc, &ai);
            if (r != VK_SUCCESS) { vk_heap_buffer = VK_NULL_HANDLE; return; }

            mapped = static_cast<uint8_t*>(ai.pMappedData);

            VkBufferDeviceAddressInfo dai{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            dai.buffer = vk_heap_buffer;
            device_address = vkGetBufferDeviceAddress(vk_dev, &dai);
        }

        DescriptorHeap::~DescriptorHeap()
        {
            VmaAllocator vma = device.get_vma_allocator();
            if (vk_heap_buffer != VK_NULL_HANDLE && vma != VK_NULL_HANDLE)
                vmaDestroyBuffer(vma, vk_heap_buffer, vma_alloc);
        }

        HAL::Descriptor DescriptorHeap::operator[](uint i)
        {
            auto THIS = static_cast<HAL::DescriptorHeap*>(this);
            return HAL::Descriptor{ *THIS, i };
        }
    }
}
