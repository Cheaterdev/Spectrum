module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:API.Queue;

import Core;
import HAL;
#undef THIS

using namespace HAL;

namespace HAL
{
    // ---- native-split members of common HAL::Queue -------------------------

    void Queue::update_tile_mappings(const update_tiling_info&) {}

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

    HAL::FenceWaiter DirectStorageQueue::execute(StorageRequest)
    {
        // Phase: Vulkan staging-buffer upload.
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

            family_idx = api_dev.get_queue_family(static_cast<int>(type));
            vk_device  = api_dev.get_native_device();

            if (family_idx == static_cast<uint32_t>(-1)) return;

            vkGetDeviceQueue(vk_device, family_idx, 0, &vk_queue);

            // Query timestamp frequency (nanoseconds per tick)
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(api_dev.get_vk_physical_dev(), &props);
            // timestampPeriod is ns/tick; we want ticks/sec for frequency
            if (props.limits.timestampPeriod > 0.0f)
                THIS->frequency = static_cast<uint64_t>(1e9 / props.limits.timestampPeriod);
        }

        void Queue::execute(const API::CommandList* list)
        {
            if (vk_queue == VK_NULL_HANDLE || !list || list->get_native() == VK_NULL_HANDLE)
                return;

            VkCommandBufferSubmitInfo cmd_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
            cmd_info.commandBuffer = list->get_native();

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
            submit.commandBufferInfoCount    = 1;
            submit.pCommandBufferInfos       = &cmd_info;
            submit.waitSemaphoreInfoCount    = wait_count;
            submit.pWaitSemaphoreInfos       = wait_count  ? &wait_info : nullptr;
            submit.signalSemaphoreInfoCount  = sig_count;
            submit.pSignalSemaphoreInfos     = sig_count ? &sig_info  : nullptr;

            std::lock_guard lock(vk_queue_mutex);
            vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE);
            // Note: no flush() — frame pacing is handled by timeline semaphores in signal().
        }

        void Queue::flush()
        {
            if (vk_queue != VK_NULL_HANDLE)
            {
                std::lock_guard lock(vk_queue_mutex);
                vkQueueWaitIdle(vk_queue);
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

            std::lock_guard lock(vk_queue_mutex);
            vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE);
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

            std::lock_guard lock(vk_queue_mutex);
            vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE);
        }

        VkQueue Queue::get_native() const { return vk_queue; }
    }
}
