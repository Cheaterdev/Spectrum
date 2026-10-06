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
    // Elements are StructureByteStride bytes, or the view format's size for raw
    // (R32_TYPELESS) and typed views, as in D3D12.
    template<class BufferView>
    static VkDescriptorBufferInfo buffer_range(const API::Resource& res, const BufferView& b, Format format)
    {
        VkDeviceSize stride = b.StructureByteStride;
        if (!stride) stride = format != Format::UNKNOWN ? format.size() / 8 : 1u;   // size() is in bits
        if (!stride) stride = 1u;
        VkDescriptorBufferInfo info{ res.get_vk_buffer() };
        info.offset = res.get_vk_buffer_offset() + static_cast<VkDeviceSize>(b.FirstElement) * stride;
        info.range  = b.NumElements ? static_cast<VkDeviceSize>(b.NumElements) * stride : VK_WHOLE_SIZE;
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
            // D3D12 MipLevels/ArraySize of -1 (or 0) mean "the rest".
            auto count = [](uint32_t n, uint32_t remaining) { return (n == 0 || n == uint32_t(-1)) ? remaining : n; };
            constexpr uint32_t ALL_MIPS = VK_REMAINING_MIP_LEVELS, ALL_LAYERS = VK_REMAINING_ARRAY_LAYERS;

            VkImageViewType type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            uint32_t first_mip = 0, mips = ALL_MIPS, first_layer = 0, layers = ALL_LAYERS;
            std::visit(overloaded{
                [&](const Views::ShaderResource::Texture2D& t)
                {
                    type = VK_IMAGE_VIEW_TYPE_2D;
                    first_mip = t.MostDetailedMip; mips = count(t.MipLevels, ALL_MIPS); layers = 1;
                },
                [&](const Views::ShaderResource::Texture2DArray& t)
                {
                    type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                    first_mip = t.MostDetailedMip; mips = count(t.MipLevels, ALL_MIPS);
                    first_layer = t.FirstArraySlice; layers = count(t.ArraySize, ALL_LAYERS);
                },
                [&](const Views::ShaderResource::Texture3D& t)
                {
                    type = VK_IMAGE_VIEW_TYPE_3D;
                    first_mip = t.MostDetailedMip; mips = count(t.MipLevels, ALL_MIPS);
                },
                [&](const Views::ShaderResource::Cube& t)
                {
                    type = VK_IMAGE_VIEW_TYPE_CUBE;
                    first_mip = t.MostDetailedMip; mips = count(t.MipLevels, ALL_MIPS); layers = 6;
                },
                [&](const Views::ShaderResource::CubeArray& t)
                {
                    type = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                    first_mip = t.MostDetailedMip; mips = count(t.MipLevels, ALL_MIPS);
                    first_layer = t.First2DArrayFace;
                    layers = (t.NumCubes == 0 || t.NumCubes == uint32_t(-1)) ? ALL_LAYERS : t.NumCubes * 6;
                },
                [&](const Views::ShaderResource::Texture1DArray& t)
                {
                    type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                    first_mip = t.MostDetailedMip; mips = count(t.MipLevels, ALL_MIPS);
                    first_layer = t.FirstArraySlice; layers = count(t.ArraySize, ALL_LAYERS);
                },
                [](auto&&) {}
            }, v.View);

            VkImageView view = type == VK_IMAGE_VIEW_TYPE_MAX_ENUM
                ? api_res.get_vk_image_view()
                : api_res.get_vk_srv_view(heap.device.get_native_device(), type, first_mip, mips, first_layer, layers);
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
            rec.buffer = buffer_range(api_res, *b, v.Format);
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
            // Array UAVs cover ArraySize slices like RWTexture2DArray; a single-layer
            // 2D view would leave every slice but the first unreachable.
            const VkDevice vk_dev = heap.device.get_native_device();
            VkImageView view = VK_NULL_HANDLE;
            auto layers = [](uint n) { return (n == 0 || n == uint(-1)) ? VK_REMAINING_ARRAY_LAYERS : n; };
            std::visit(overloaded{
                [&](const Views::UnorderedAccess::Texture2D& t)      { view = api_res.get_vk_uav_view(vk_dev, t.MipSlice, 0); },
                [&](const Views::UnorderedAccess::Texture3D& t)      { view = api_res.get_vk_uav_view(vk_dev, t.MipSlice, 0); },
                [&](const Views::UnorderedAccess::Texture2DArray& t)
                {
                    view = api_res.get_vk_srv_view(vk_dev, VK_IMAGE_VIEW_TYPE_2D_ARRAY, t.MipSlice, 1, t.FirstArraySlice, layers(t.ArraySize));
                },
                [&](const Views::UnorderedAccess::Texture1DArray& t)
                {
                    view = api_res.get_vk_srv_view(vk_dev, VK_IMAGE_VIEW_TYPE_1D_ARRAY, t.MipSlice, 1, t.FirstArraySlice, layers(t.ArraySize));
                },
                [&](auto&&) { view = api_res.get_vk_uav_view(vk_dev, 0, 0); }
            }, v.View);
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
                rec.buffer = buffer_range(api_res, *b, v.Format);
                if (b->CounterResource)
                {
                    auto& counter = static_cast<const API::Resource&>(*b->CounterResource);
                    rec.counter = { counter.get_vk_buffer(), counter.get_vk_buffer_offset() + b->CounterOffsetInBytes, sizeof(uint32_t) };
                }
            }
            else
                rec.buffer = { api_res.get_vk_buffer(), api_res.get_vk_buffer_offset(), VK_WHOLE_SIZE };
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
        rec.buffer = { api_res.get_vk_buffer(), api_res.get_vk_buffer_offset() + v.OffsetInBytes,
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
            if (vk_set != VK_NULL_HANDLE)
                counter_bound.resize(capacity);
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

            // Binding 2 is a separate array from binding 0: a slot reused for a
            // counter-less view keeps the previous view's counter descriptor,
            // which can name a freed buffer, unless it is nulled here.
            const bool has_counter   = record.counter.buffer != VK_NULL_HANDLE;
            const bool clear_counter = !has_counter && counter_bound[slot] && device.supports_null_descriptor();
            const VkDescriptorBufferInfo null_counter{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

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

            if (has_counter || clear_counter)
            {
                auto& c = writes[count++];
                c.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                c.dstSet          = vk_set;
                c.dstBinding      = 2;
                c.dstArrayElement = slot;
                c.descriptorCount = 1;
                c.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                c.pBufferInfo     = has_counter ? &record.counter : &null_counter;
            }

            std::lock_guard g(write_mutex);
            vkUpdateDescriptorSets(device.get_native_device(), count, writes, 0, nullptr);
            if (has_counter || clear_counter)
                counter_bound[slot] = has_counter;
        }

        HAL::Descriptor DescriptorHeap::operator[](uint i)
        {
            auto THIS = static_cast<HAL::DescriptorHeap*>(this);
            return HAL::Descriptor{ *THIS, i };
        }
    }
}
