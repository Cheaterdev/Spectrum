module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <fstream>
module HAL:API.Queue;

import Core;
import HAL;
#undef THIS

using namespace HAL;

namespace HAL
{
    // ---- native-split members of common HAL::Queue -------------------------

    void Queue::update_tile_mappings(const update_tiling_info& infos)
    {
        PROFILE(L"update_tile_mappings");
        ASSERT(!infos.source && "copy_mappings (CopyTileMappings) has no Vulkan implementation");

        auto* res = static_cast<API::Resource*>(infos.resource);
        ASSERT(res->is_sparse() && "tile mapping on a resource that is not sparse");
        const auto& layout = res->get_sparse_layout();
        const auto& desc   = infos.resource->get_desc();
        constexpr auto tile_size = API::Device::SPARSE_TILE_SIZE;

        std::vector<VkSparseMemoryBind>      opaque;   // buffers, and image mip tails
        std::vector<VkSparseImageMemoryBind> image;

        for (auto& [heap, tiles] : infos.tiles)
        {
            // A null heap unmaps (D3D12_TILE_RANGE_FLAG_NULL).
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkDeviceSize   base   = 0;
            if (heap)
                std::tie(memory, base) = heap->get_sparse_memory();

            for (const ResourceTile& tile : tiles)
            {
                const auto& hp = tile.heap_position;
                const VkDeviceSize memory_offset = memory ? base + VkDeviceSize(hp.offset) * tile_size : 0;

                if (desc.is_buffer())
                {
                    opaque.push_back({ VkDeviceSize(tile.pos.x) * tile_size, VkDeviceSize(hp.count) * tile_size,
                                       memory, memory_offset, 0 });
                    continue;
                }

                const auto& tex  = desc.as_texture();
                const uint mip   = tile.subresource % tex.MipLevels;
                const uint layer = tile.subresource / tex.MipLevels;

                if (mip >= layout.mip_tail_first)
                {
                    const VkDeviceSize tail = layout.mip_tail_offset
                        + (layout.single_mip_tail ? 0 : VkDeviceSize(layer) * layout.mip_tail_stride);
                    opaque.push_back({ tail, layout.mip_tail_size, memory, memory_offset, 0 });
                    continue;
                }

                const uint3 extent = uint3::max({ 1, 1, 1 }, uint3(
                    tex.Dimensions.x >> mip,
                    tex.is1D() ? 1u : tex.Dimensions.y >> mip,
                    tex.is3D() ? tex.Dimensions.z >> mip : 1u));
                const VkExtent3D g = layout.granularity;
                const uint3 count = { (extent.x + g.width - 1) / g.width,
                                      (extent.y + g.height - 1) / g.height,
                                      (extent.z + g.depth - 1) / g.depth };

                // hp.count tiles from tile.pos, in the x-y-z order D3D12 uses for a
                // box-less tile region.
                const uint start = tile.pos.x + tile.pos.y * count.x + tile.pos.z * count.x * count.y;
                for (uint i = 0; i < hp.count; i++)
                {
                    const uint linear = start + i;
                    const uint3 pos = { linear % count.x, (linear / count.x) % count.y, linear / (count.x * count.y) };

                    VkSparseImageMemoryBind b{};
                    b.subresource  = { layout.aspect, mip, layer };
                    b.offset       = { int(pos.x * g.width), int(pos.y * g.height), int(pos.z * g.depth) };
                    b.extent       = { std::min(g.width,  extent.x - pos.x * g.width),
                                       std::min(g.height, extent.y - pos.y * g.height),
                                       std::min(g.depth,  extent.z - pos.z * g.depth) };
                    b.memory       = memory;
                    b.memoryOffset = memory ? memory_offset + VkDeviceSize(i) * tile_size : 0;
                    image.push_back(b);
                }
            }
        }

        if (opaque.empty() && image.empty()) return;

        VkSparseBufferMemoryBindInfo      buffer_bind{ res->get_vk_buffer(), uint32_t(opaque.size()), opaque.data() };
        VkSparseImageOpaqueMemoryBindInfo opaque_bind{ res->get_vk_image(), uint32_t(opaque.size()), opaque.data() };
        VkSparseImageMemoryBindInfo       image_bind { res->get_vk_image(), uint32_t(image.size()),  image.data() };

        VkBindSparseInfo info{ VK_STRUCTURE_TYPE_BIND_SPARSE_INFO };
        if (desc.is_buffer())
        {
            info.bufferBindCount = 1;
            info.pBufferBinds    = &buffer_bind;
        }
        else
        {
            info.imageOpaqueBindCount = opaque.empty() ? 0 : 1;
            info.pImageOpaqueBinds    = &opaque_bind;
            info.imageBindCount       = image.empty() ? 0 : 1;
            info.pImageBinds          = &image_bind;
        }
        bind_sparse(info);
    }

    ClockCalibrationInfo Queue::get_clock_time() const
    {
        // Phase: vkGetCalibratedTimestampsEXT
        return { 0, 0, frequency };
    }

    // ---- HAL::DirectStorageQueue (streaming) --------------------------------

    DirectStorageQueue::DirectStorageQueue(Device& device) : device(device), requestCounter(device) {}
    DirectStorageQueue::~DirectStorageQueue() { executor.stop_and_wait(); }
    void DirectStorageQueue::flush() {}
    void DirectStorageQueue::stop_all() {}

    HAL::FenceWaiter DirectStorageQueue::signal()
    {
        auto value = ++m_fenceValue;
        requestCounter.signal(value);
        return FenceWaiter{ &requestCounter, value };
    }

    void DirectStorageQueue::signal_and_wait() { auto s = signal(); flush(); s.wait(); }
    bool DirectStorageQueue::is_complete(UINT64 fence) { return requestCounter.get_completed_value() >= fence; }
    FenceWaiter DirectStorageQueue::get_waiter() { return FenceWaiter{ &requestCounter, 0 }; }

    HAL::FenceWaiter DirectStorageQueue::execute(StorageRequest srequest)
    {
        // Vulkan has no DirectStorage — read the cache file and upload via a
        // staging-buffer command list.  The binary data was written by
        // TextureResource::read_range with each subresource's no-box
        // get_texture_layout; the same layouts are used here so the source row
        // strides match what was stored.  update_texture() handles the mismatch
        // between that stride and the 256-byte-aligned upload stride internally.

        std::ifstream file(srequest.file, std::ios::binary);
        if (!file.is_open())
            return signal();

        file.seekg(static_cast<std::streamoff>(srequest.file_offset), std::ios::beg);
        std::vector<std::byte> data(srequest.size);
        file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(srequest.size));
        if (!file)
            return signal();

        // Vulkan compress() is a no-op, so the file data is always uncompressed.
        // (srequest.compressed will be false; nothing to decompress.)

        auto list = device.get_upload_list();

        std::visit(overloaded{
            [&](const StorageRequest::Buffer& buffer)
            {
                // Raw bytes, never compressed here; copied through a staging buffer into the range
                // the request names.
                list->get_copy().update_buffer(srequest.resource.get(), buffer.offset,
                    reinterpret_cast<const char*>(data.data()), data.size());
            },
            [&](const StorageRequest::Texture& texture)
            {
                // One request carries `count` subresources (a whole mip chain),
                // packed by get_texture_range_layout as TextureResource::read_range
                // saved them; each keeps its own no-box layout's row stride.
                const auto& desc  = srequest.resource->get_desc();
                const auto& tdesc = desc.as_texture();
                const auto  range = device.get_texture_range_layout(desc, texture.subresource, texture.count);
                for (uint i = 0; i < texture.count; ++i)
                {
                    const uint sub = texture.subresource + i;
                    auto save_layout = device.get_texture_layout(desc, sub);
                    auto box = ivec3(tdesc.get_size(tdesc.get_mip(sub)));

                    list->get_copy().update_texture(
                        srequest.resource.get(),
                        ivec3(0, 0, 0),
                        box,
                        sub,
                        reinterpret_cast<const char*>(data.data() + range.subresource_offsets[i]),
                        save_layout.row_stride,
                        save_layout.slice_stride);
                }
            },
            [](auto) { ASSERT(false); std::unreachable(); }
        }, srequest.operation);

        list->execute_and_wait();
        return signal();
    }

    // ---- HAL::API::Queue ---------------------------------------------------

    namespace API
    {
        void Queue::construct(HAL::CommandListType type, Device* device)
        {
            auto THIS = static_cast<HAL::Queue*>(this);

            auto& api_dev = *static_cast<API::Device*>(device);
            if (api_dev.get_native_device() == VK_NULL_HANDLE) return;

            m_device   = &api_dev;
            family_idx = api_dev.get_queue_family(static_cast<int>(type));
            vk_device  = api_dev.get_native_device();

            if (family_idx == static_cast<uint32_t>(-1)) return;

            vkGetDeviceQueue(vk_device, family_idx, api_dev.get_queue_index(static_cast<int>(type)), &vk_queue);
            vk_queue_mutex = &api_dev.get_queue_mutex(static_cast<int>(type));

            const int bind_type = api_dev.queue_supports_sparse(static_cast<int>(type))
                ? static_cast<int>(type) : static_cast<int>(HAL::CommandListType::DIRECT);
            ASSERT(api_dev.queue_supports_sparse(bind_type) && "the DIRECT queue family cannot bind sparse memory");
            vkGetDeviceQueue(vk_device, api_dev.get_queue_family(bind_type), api_dev.get_queue_index(bind_type), &sparse_queue);
            sparse_queue_mutex = &api_dev.get_queue_mutex(bind_type);

            VkSemaphoreTypeCreateInfo sem_type{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
            sem_type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            VkSemaphoreCreateInfo sem_ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            sem_ci.pNext = &sem_type;
            vkCreateSemaphore(vk_device, &sem_ci, nullptr, &sparse_timeline);

            if (auto set_name = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                    vkGetDeviceProcAddr(vk_device, "vkSetDebugUtilsObjectNameEXT")))
            {
                auto name = [&](VkSemaphore s, std::string label) {
                    VkDebugUtilsObjectNameInfoEXT ni{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
                    ni.objectType   = VK_OBJECT_TYPE_SEMAPHORE;
                    ni.objectHandle = reinterpret_cast<uint64_t>(s);
                    ni.pObjectName  = label.c_str();
                    set_name(vk_device, &ni);
                };
                const std::string queue_name(magic_enum::enum_name(type));
                name(THIS->commandListCounter.timeline_semaphore, queue_name + " fence");
                name(sparse_timeline, queue_name + " sparse timeline");
            }

            // Query timestamp frequency (nanoseconds per tick)
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(api_dev.get_vk_physical_dev(), &props);
            // timestampPeriod is ns/tick; we want ticks/sec for frequency
            if (props.limits.timestampPeriod > 0.0f)
                THIS->frequency = static_cast<uint64_t>(1e9 / props.limits.timestampPeriod);
        }

        Queue::~Queue()
        {
            if (init_pool != VK_NULL_HANDLE)
                vkDestroyCommandPool(vk_device, init_pool, nullptr);
            if (sparse_timeline != VK_NULL_HANDLE)
                vkDestroySemaphore(vk_device, sparse_timeline, nullptr);
        }

        // Unlike D3D12's UpdateTileMappings, vkQueueBindSparse is not ordered
        // against command buffers submitted to the same queue. Without the
        // timeline fences on both sides, a remap can land while earlier work still
        // reads the old tiles, or after later work has already read unmapped ones.
        void Queue::bind_sparse(VkBindSparseInfo& info)
        {
            if (vk_queue == VK_NULL_HANDLE) return;

            std::unique_lock own(*vk_queue_mutex, std::defer_lock);
            std::unique_lock bind(*sparse_queue_mutex, std::defer_lock);
            if (sparse_queue_mutex == vk_queue_mutex)
                own.lock();
            else
                std::lock(own, bind);

            const uint64_t prior = ++sparse_value;
            const uint64_t bound = ++sparse_value;

            auto submit = [&](bool wait, uint64_t value) {
                VkSemaphoreSubmitInfo sem{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                sem.semaphore = sparse_timeline;
                sem.value     = value;
                sem.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
                submit.waitSemaphoreInfoCount   = wait ? 1 : 0;
                submit.pWaitSemaphoreInfos      = wait ? &sem : nullptr;
                submit.signalSemaphoreInfoCount = wait ? 0 : 1;
                submit.pSignalSemaphoreInfos    = wait ? nullptr : &sem;
                check_device_lost(vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE), "vkQueueSubmit2 (sparse fence)");
            };

            submit(false, prior);

            VkTimelineSemaphoreSubmitInfo timeline{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
            timeline.waitSemaphoreValueCount   = 1;
            timeline.pWaitSemaphoreValues      = &prior;
            timeline.signalSemaphoreValueCount = 1;
            timeline.pSignalSemaphoreValues    = &bound;
            info.pNext                = &timeline;
            info.waitSemaphoreCount   = 1;
            info.pWaitSemaphores      = &sparse_timeline;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores    = &sparse_timeline;
            check_device_lost(vkQueueBindSparse(sparse_queue, 1, &info, VK_NULL_HANDLE), "vkQueueBindSparse");

            submit(true, bound);
        }

        VkCommandBuffer Queue::flush_init_transitions()
        {
            if (!m_device) return VK_NULL_HANDLE;

            auto barriers = m_device->take_pending_init_transitions();
            if (barriers.empty()) return VK_NULL_HANDLE;

            if (init_pool == VK_NULL_HANDLE)
            {
                VkCommandPoolCreateInfo pci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
                pci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                                     | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                pci.queueFamilyIndex = family_idx;
                vkCreateCommandPool(vk_device, &pci, nullptr, &init_pool);

                VkCommandBufferAllocateInfo cai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
                cai.commandPool        = init_pool;
                cai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                cai.commandBufferCount = static_cast<uint32_t>(init_cbs.size());
                vkAllocateCommandBuffers(vk_device, &cai, init_cbs.data());
            }

            VkCommandBuffer cb = init_cbs[init_cb_index];
            init_cb_index = (init_cb_index + 1) % static_cast<uint32_t>(init_cbs.size());

            vkResetCommandBuffer(cb, 0);
            VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cb, &bi);

            VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            dep.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
            dep.pImageMemoryBarriers    = barriers.data();
            vkCmdPipelineBarrier2(cb, &dep);

            vkEndCommandBuffer(cb);
            return cb;
        }

        void Queue::execute(const API::CommandList* list)
        {
            if (vk_queue == VK_NULL_HANDLE || !list || list->get_native() == VK_NULL_HANDLE)
                return;

            // Flush pending initial-layout transitions ahead of the real work so
            // fresh images actually rest in the layout the state manager assumes.
            VkCommandBufferSubmitInfo cmd_infos[2]{};
            uint32_t cmd_count = 0;

            if (VkCommandBuffer init_cb = flush_init_transitions(); init_cb != VK_NULL_HANDLE)
            {
                cmd_infos[cmd_count] = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
                cmd_infos[cmd_count].commandBuffer = init_cb;
                ++cmd_count;
            }

            VkCommandBufferSubmitInfo& cmd_info = cmd_infos[cmd_count];
            cmd_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
            cmd_info.commandBuffer = list->get_native();
            ++cmd_count;

            // Consume pending acquire/present semaphores (set once per frame by SwapChain).
            VkSemaphoreSubmitInfo wait_info{}, sig_info{};
            uint32_t wait_count = 0, sig_count = 0;

            if (pending_wait_sem != VK_NULL_HANDLE)
            {
                wait_info = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                wait_info.semaphore = pending_wait_sem;
                wait_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                wait_count = 1;
                pending_wait_sem = VK_NULL_HANDLE;
            }
            if (pending_signal_sem != VK_NULL_HANDLE)
            {
                sig_info = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                sig_info.semaphore = pending_signal_sem;
                sig_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
                sig_count = 1;
                pending_signal_sem = VK_NULL_HANDLE;
            }

            VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
            submit.commandBufferInfoCount    = cmd_count;
            submit.pCommandBufferInfos       = cmd_infos;
            submit.waitSemaphoreInfoCount    = wait_count;
            submit.pWaitSemaphoreInfos       = wait_count  ? &wait_info : nullptr;
            submit.signalSemaphoreInfoCount  = sig_count;
            submit.pSignalSemaphoreInfos     = sig_count ? &sig_info  : nullptr;

            std::lock_guard lock(*vk_queue_mutex);
            check_device_lost(vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE), "vkQueueSubmit2 (execute)");
            // Note: no flush() — frame pacing is handled by timeline semaphores in signal().
        }

        void Queue::flush()
        {
            if (vk_queue != VK_NULL_HANDLE)
            {
                std::lock_guard lock(*vk_queue_mutex);
                check_device_lost(vkQueueWaitIdle(vk_queue), "vkQueueWaitIdle");
            }
        }

        void Queue::signal(Fence& fence, Fence::CounterType value)
        {
            if (vk_queue == VK_NULL_HANDLE || fence.timeline_semaphore == VK_NULL_HANDLE)
                return;

            VkSemaphoreSubmitInfo sem{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
            sem.semaphore = fence.timeline_semaphore;
            sem.value     = value;
            sem.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
            submit.signalSemaphoreInfoCount = 1;
            submit.pSignalSemaphoreInfos    = &sem;

            std::lock_guard lock(*vk_queue_mutex);
            check_device_lost(vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE), "vkQueueSubmit2 (signal)");
        }

        void Queue::gpu_wait(HAL::FenceWaiter waiter)
        {
            if (!waiter || vk_queue == VK_NULL_HANDLE) return;
            if (waiter.fence->timeline_semaphore == VK_NULL_HANDLE) return;

            VkSemaphoreSubmitInfo sem{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
            sem.semaphore = waiter.fence->timeline_semaphore;
            sem.value     = waiter.value;
            sem.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
            submit.waitSemaphoreInfoCount = 1;
            submit.pWaitSemaphoreInfos    = &sem;

            std::lock_guard lock(*vk_queue_mutex);
            check_device_lost(vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE), "vkQueueSubmit2 (gpu_wait)");
        }

        VkQueue Queue::get_native() const { return vk_queue; }
    }
}
