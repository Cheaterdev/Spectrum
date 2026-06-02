export module HAL:API.Fence;
import vulkan;

export namespace HAL
{
    namespace API
    {
        class Fence
        {
            // Queue signals / waits on the timeline semaphore directly.
            friend class Queue;

        protected:
            VkSemaphore  timeline_semaphore = VK_NULL_HANDLE;
            VkDevice     device             = VK_NULL_HANDLE;

        public:
            using CounterType = uint64_t;
        };

        class Event
        {
            friend class Queue;

        protected:
            // CPU-side wait uses a binary VkFence on Vulkan.
            VkFence vk_fence     = VK_NULL_HANDLE;
            // Kept for interface compat with common HAL code.
            HANDLE  m_fenceEvent = nullptr;

        public:
            virtual ~Event() = default;
        };
    }
}
