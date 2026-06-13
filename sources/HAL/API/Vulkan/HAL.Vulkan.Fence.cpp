module HAL:Fence;

import vulkan;
import Core;
import :Device;

// Vulkan implementation of the HAL::Fence / HAL::Event wrappers using a
// VK_KHR_timeline_semaphore.  This is inherently native code (no shared
// orchestration), so it lives per-backend — the D3D12 equivalent is
// D3D12/HAL.D3D12.Fence.cpp.

namespace HAL
{
    // On Vulkan we synchronise via timeline semaphores (vkWaitSemaphores),
    // so the Win32-event abstraction is unused — these are no-ops.
    Event::Event() {}
    Event::~Event() {}
    void Event::wait() {}

    Fence::~Fence()
    {
        if (timeline_semaphore != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, timeline_semaphore, nullptr);
            timeline_semaphore = VK_NULL_HANDLE;
        }
    }

    Fence::Fence(Device& device_)
    {
        device = device_.get_native_device();

        VkSemaphoreTypeCreateInfo type_info{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
        type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        type_info.initialValue  = 0;

        VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        info.pNext = &type_info;

        vkCreateSemaphore(device, &info, nullptr, &timeline_semaphore);
    }

    void Fence::signal(CounterType value)
    {
        // Host-side (CPU) signal of the timeline semaphore.
        VkSemaphoreSignalInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO };
        info.semaphore = timeline_semaphore;
        info.value     = value;
        vkSignalSemaphore(device, &info);
    }

    Fence::CounterType Fence::get_completed_value() const
    {
        uint64_t value = 0;
        vkGetSemaphoreCounterValue(device, timeline_semaphore, &value);
        return value;
    }

    void Fence::wait(CounterType value) const
    {
        if (get_completed_value() >= value) return;

        PROFILE(L"FenceWait");
        VkSemaphoreWaitInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
        info.semaphoreCount = 1;
        info.pSemaphores    = &timeline_semaphore;
        info.pValues        = &value;
        vkWaitSemaphores(device, &info, std::numeric_limits<uint64>::max());
    }
}
