module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:DescriptorHeap;

import :Debug;
import :Resource;
import :Resource.Buffer;
import :API.Device;   // get_native_device(), bindless layouts and capacities

import vulkan;
import Core;

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

    // StructuredBuffers are sub-ranges (FirstElement..NumElements) of shared
    // buffers; honour them or the shader reads the wrong elements.
    template<class BufferView>
    static VkDescriptorBufferInfo buffer_range(VkBuffer buffer, const BufferView& b)
    {
        const VkDeviceSize stride = b.StructureByteStride ? b.StructureByteStride : 1u;
        VkDescriptorBufferInfo info{ buffer };
        info.offset = static_cast<VkDeviceSize>(b.FirstElement) * stride;
        info.range  = b.NumElements ? static_cast<VkDeviceSize>(b.NumElements) * stride : VK_WHOLE_SIZE;

        // TEMP: storage-buffer alignment investigation -- remove once fixed.
        if (info.offset % 16)
        {
            static std::mutex m; static int n = 0;
            std::lock_guard g(m);
            if (n++ < 40)
                std::ofstream("misaligned_sb.temp", std::ios::app) << "offset " << info.offset << " stride " << b.StructureByteStride
                    << " first " << b.FirstElement << " count " << b.NumElements << "\n" << std::stacktrace::current() << "\n----\n";
        }
        return info;
    }

    // D3D12 null view (a view with no resource): reads return zero, writes are
    // dropped. Left unwritten, the slot holds garbage or a stale descriptor, and a
    // shader reading it -- get_null_descriptor() hands these out for every unbound
    // table slot -- faults the device (VK_ERROR_DEVICE_LOST, WRITE_INVALID).
    static void place_null(API::DescriptorHeap& heap, uint slot, VkDescriptorType type)
    {
        ASSERT(heap.device.supports_null_descriptor() && "null view on a device without nullDescriptor: slot stays unwritten");
        if (!heap.device.supports_null_descriptor()) return;

        API::DescriptorRecord rec;
        rec.type = type;
        if (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
            rec.image = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
        else
            rec.buffer = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
        heap.store(slot, rec);
    }

    void Descriptor::place(const Views::ShaderResource& v, bool /*skip_gpu_write*/)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!v.Resource)
        {
            place_null(api_heap, offset, std::holds_alternative<Views::ShaderResource::Buffer>(v.View)
                ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
            return;
        }
        auto& api_res = static_cast<const API::Resource&>(*v.Resource);

        API::DescriptorRecord rec;
        if (api_res.get_vk_image() != VK_NULL_HANDLE)
        {
            // GENERAL matches to_native(SHADER_RESOURCE), so SRV and UAV slots for
            // the same image agree on its layout.
            VkImageView view = api_res.get_vk_image_view();
            ASSERT(view != VK_NULL_HANDLE && "SRV: image has no view");
            if (view == VK_NULL_HANDLE) return;
            rec.type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            rec.image = { VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL };
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            auto* b = std::get_if<Views::ShaderResource::Buffer>(&v.View);
            ASSERT(b && "SRV: buffer resource with a non-buffer view");
            if (!b) return;
            rec.type   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            rec.buffer = buffer_range(api_res.get_vk_buffer(), *b);
        }
        else
        {
            ASSERT(!"SRV: resource has neither a VkImage nor a VkBuffer (failed creation?)");
            return;
        }

        api_heap.store(offset, rec);
    }

    void Descriptor::place(const Views::UnorderedAccess& v, bool /*skip_gpu_write*/)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!v.Resource)
        {
            place_null(api_heap, offset, std::holds_alternative<Views::UnorderedAccess::Buffer>(v.View)
                ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            return;
        }
        auto& api_res = static_cast<const API::Resource&>(*v.Resource);

        API::DescriptorRecord rec;
        if (api_res.get_vk_image() != VK_NULL_HANDLE)
        {
            uint32_t mip_slice = 0, array_slice = 0;
            std::visit(overloaded{
                [&](const Views::UnorderedAccess::Texture2D& t)      { mip_slice = t.MipSlice; },
                [&](const Views::UnorderedAccess::Texture2DArray& t) { mip_slice = t.MipSlice; array_slice = t.FirstArraySlice; },
                [&](const Views::UnorderedAccess::Texture3D& t)      { mip_slice = t.MipSlice; },
                [](auto&&) {}
            }, v.View);

            VkImageView view = api_res.get_vk_uav_view(heap.device.get_native_device(), mip_slice, array_slice);
            ASSERT(view != VK_NULL_HANDLE && "UAV: storage image view creation failed");
            if (view == VK_NULL_HANDLE) return;
            rec.type  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            rec.image = { VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL };
        }
        else if (api_res.get_vk_buffer() != VK_NULL_HANDLE)
        {
            rec.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            if (auto* b = std::get_if<Views::UnorderedAccess::Buffer>(&v.View))
            {
                rec.buffer = buffer_range(api_res.get_vk_buffer(), *b);
                if (b->CounterResource)
                {
                    auto& counter = static_cast<const API::Resource&>(*b->CounterResource);
                    rec.counter = { counter.get_vk_buffer(), b->CounterOffsetInBytes, sizeof(uint32_t) };
                }
            }
            else
                rec.buffer = { api_res.get_vk_buffer(), 0, VK_WHOLE_SIZE };
        }
        else
        {
            ASSERT(!"UAV: resource has neither a VkImage nor a VkBuffer (failed creation?)");
            return;
        }

        api_heap.store(offset, rec);
    }

    void Descriptor::place(const Views::ConstantBuffer& v, bool /*skip_gpu_write*/)
    {
        auto& api_heap = static_cast<API::DescriptorHeap&>(heap);
        if (!v.Resource)
        {
            place_null(api_heap, offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            return;
        }
        auto& api_res = static_cast<const API::Resource&>(*v.Resource);
        ASSERT(api_res.get_vk_buffer() != VK_NULL_HANDLE && "CBV: resource has no VkBuffer");
        if (api_res.get_vk_buffer() == VK_NULL_HANDLE) return;

        // CBVs are sub-allocated inside larger shared buffers; honour
        // OffsetInBytes/SizeInBytes or the shader reads the wrong sub-region.
        API::DescriptorRecord rec;
        rec.type   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        rec.buffer = { api_res.get_vk_buffer(), v.OffsetInBytes,
                       v.SizeInBytes > 0 ? VkDeviceSize(v.SizeInBytes) : VK_WHOLE_SIZE };
        api_heap.store(offset, rec);
    }

    void Descriptor::place(const Views::RenderTarget&, bool) {} // handled by dynamic rendering
    void Descriptor::place(const Views::DepthStencil&, bool) {} // handled by dynamic rendering

    void Descriptor::operator=(const Descriptor& r)
    {
        // D3D12 CopyDescriptors.  RTV/DSV heaps have no descriptors (dynamic
        // rendering) -- the ResourceInfo copy done by HAL::Handle::place() is
        // enough there.
        auto& dst_api = static_cast<API::DescriptorHeap&>(heap);
        auto& src_api = static_cast<API::DescriptorHeap&>(r.heap);

        // RTV/DSV heaps keep no records (get_record is null); any other heap's
        // source slot must have been placed, or the copy spreads an unwritten slot.
        if (auto* rec = src_api.get_record(r.offset))
        {
            ASSERT(!rec->empty() && "descriptor copy from a slot that was never placed");
            if (!rec->empty())
                dst_api.store(offset, *rec);
        }
    }

    uint DescriptorHeap::get_size() { return capacity; }

    namespace API
    {
        DescriptorHeap::DescriptorHeap(Device& dev, const DescriptorHeapDesc& d)
            : device(dev), desc(d)
        {
            capacity = d.Count;

            // RTV / DSV heaps have no descriptors (dynamic rendering).
            if (d.HeapType == DescriptorHeapType::RTV || d.HeapType == DescriptorHeapType::DSV)
                return;

            const bool is_sampler = d.HeapType == DescriptorHeapType::SAMPLER;
            if (check(d.Flags & DescriptorHeapFlags::ShaderVisible))
            {
                VkDevice vk_dev = dev.get_native_device();

                // Pool sizes must cover the layout's full binding counts, which are
                // the device capacities -- the heap can only address up to those.
                const uint32_t layout_count = is_sampler ? dev.get_sampler_heap_capacity()
                                                         : dev.get_resource_heap_capacity();
                capacity = std::min(d.Count, layout_count);

                std::vector<VkDescriptorPoolSize>           sizes;
                std::vector<VkMutableDescriptorTypeListEXT> lists;
                if (is_sampler)
                    sizes = { { VK_DESCRIPTOR_TYPE_SAMPLER, layout_count } };
                else
                {
                    sizes = {
                        { VK_DESCRIPTOR_TYPE_MUTABLE_EXT,    layout_count },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, layout_count },  // Append/Consume counters
                        { VK_DESCRIPTOR_TYPE_SAMPLER,        Device::get_inline_sampler_count() },
                    };
                    lists.resize(sizes.size());
                    lists[0].descriptorTypeCount = static_cast<uint32_t>(std::size(Device::mutable_resource_types));
                    lists[0].pDescriptorTypes    = Device::mutable_resource_types;
                }

                VkMutableDescriptorTypeCreateInfoEXT mutable_ci{ VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT };
                mutable_ci.mutableDescriptorTypeListCount = static_cast<uint32_t>(lists.size());
                mutable_ci.pMutableDescriptorTypeLists    = lists.data();

                VkDescriptorPoolCreateInfo pool_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
                pool_ci.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
                pool_ci.maxSets       = 1;
                pool_ci.poolSizeCount = static_cast<uint32_t>(sizes.size());
                pool_ci.pPoolSizes    = sizes.data();
                pool_ci.pNext         = is_sampler ? nullptr : &mutable_ci;
                dev.process_result(vkCreateDescriptorPool(vk_dev, &pool_ci, nullptr, &vk_pool), "descriptor pool");

                VkDescriptorSetLayout layout = is_sampler ? dev.get_sampler_set_layout()
                                                          : dev.get_resource_set_layout();
                VkDescriptorSetAllocateInfo alloc{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
                alloc.descriptorPool     = vk_pool;
                alloc.descriptorSetCount = 1;
                alloc.pSetLayouts        = &layout;
                dev.process_result(vkAllocateDescriptorSets(vk_dev, &alloc, &vk_set), "descriptor set");
            }

            records.resize(capacity);
        }

        DescriptorHeap::~DescriptorHeap()
        {
            // Destroying the pool frees its set.
            if (vk_pool != VK_NULL_HANDLE)
                vkDestroyDescriptorPool(device.get_native_device(), vk_pool, nullptr);
        }

        void DescriptorHeap::store(uint slot, const DescriptorRecord& record)
        {
            ASSERT(slot < records.size() && "descriptor slot beyond the heap's capacity");
            if (slot >= records.size()) return;
            records[slot] = record;
            if (vk_set == VK_NULL_HANDLE) return;

            VkWriteDescriptorSet writes[2]{};
            uint32_t count = 0;

            auto& w = writes[count++];
            w.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            w.dstSet          = vk_set;
            w.dstBinding      = 0;
            w.dstArrayElement = slot;
            w.descriptorCount = 1;
            w.descriptorType  = record.type;
            if (record.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || record.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                w.pImageInfo  = &record.image;
            else
                w.pBufferInfo = &record.buffer;

            if (record.counter.buffer != VK_NULL_HANDLE)
            {
                auto& c = writes[count++];
                c.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                c.dstSet          = vk_set;
                c.dstBinding      = 2;
                c.dstArrayElement = slot;
                c.descriptorCount = 1;
                c.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                c.pBufferInfo     = &record.counter;
            }

            std::lock_guard g(write_mutex);
            vkUpdateDescriptorSets(device.get_native_device(), count, writes, 0, nullptr);
        }

        HAL::Descriptor DescriptorHeap::operator[](uint i)
        {
            auto THIS = static_cast<HAL::DescriptorHeap*>(this);
            return HAL::Descriptor{ *THIS, i };
        }
    }
}
