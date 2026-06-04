export module HAL:API.Queue;
import vulkan;
import :Types;
import :Utils;
import :Fence;
import :CommandList;

export namespace HAL
{
    namespace API
    {
        class Queue
        {
        protected:
            VkQueue       vk_queue   = VK_NULL_HANDLE;
            VkDevice      vk_device  = VK_NULL_HANDLE;
            uint32_t      family_idx = std::numeric_limits<uint32_t>::max();

            // Acquire / present semaphores consumed once on the next execute().
            // Set by SwapChain after vkAcquireNextImageKHR; cleared by execute().
            VkSemaphore   pending_wait_sem   = VK_NULL_HANDLE;
            VkSemaphore   pending_signal_sem = VK_NULL_HANDLE;

            // Protects all vkQueue* calls — VkQueue must be externally synchronized.
            mutable std::mutex vk_queue_mutex;

            void execute(const API::CommandList* list);
            void flush();
            void signal(Fence& fence, Fence::CounterType value);
            void gpu_wait(HAL::FenceWaiter waiter);
            void construct(HAL::CommandListType type, Device* device);

        public:
            virtual ~Queue() = default;

            VkQueue get_native() const;

            // Called by SwapChain to wire the per-frame binary semaphores into
            // the next command buffer submission.
            void set_frame_semaphores(VkSemaphore wait, VkSemaphore signal) noexcept
            {
                pending_wait_sem   = wait;
                pending_signal_sem = signal;
            }

            // Thread-safe present: takes the queue mutex so submit and present
            // cannot race.  Returns the VkResult from vkQueuePresentKHR.
            VkResult present(VkPresentInfoKHR& pi) noexcept
            {
                if (vk_queue == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
                std::lock_guard lock(vk_queue_mutex);
                return vkQueuePresentKHR(vk_queue, &pi);
            }

            // Thread-safe raw submit used by SwapChain for its transition CB.
            void submit_raw(VkSubmitInfo2& info) noexcept
            {
                if (vk_queue == VK_NULL_HANDLE) return;
                std::lock_guard lock(vk_queue_mutex);
                vkQueueSubmit2(vk_queue, 1, &info, VK_NULL_HANDLE);
            }
        };

        // DirectStorage is D3D12-specific; provide an empty stub so
        // HAL::DirectStorageQueue (which inherits from this) still compiles.
        class DirectStorageQueue
        {
        protected:
        public:
            virtual ~DirectStorageQueue() = default;
        };
    }
}
