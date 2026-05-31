export module HAL:API.Fence;
import vulkan;

export namespace HAL
{
    namespace API
    {
        class Fence
        {
        public:
            VkSemaphore  timeline_semaphore = VK_NULL_HANDLE; // VK_KHR_timeline_semaphore
            VkDevice     device = VK_NULL_HANDLE;             // needed for signal/wait (unlike D3D12's self-contained fence)
            using CounterType = uint64_t;
        };

        class Event
        {
        public:
            // On Vulkan, CPU-side waits use VkFence (binary) rather than a Win32 event.
            VkFence vk_fence = VK_NULL_HANDLE;
            // Keep HANDLE for interface compat with common HAL code that stores it.
            HANDLE m_fenceEvent = nullptr;
        };
    }
}
