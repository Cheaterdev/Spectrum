export module HAL:API.CommandAllocator;
import Core;
import vulkan;
import :Types;
import :Utils;

export namespace HAL
{
    namespace API
    {
        class CommandAllocator
        {
        protected:
        public:
            // Vulkan: command buffers are allocated from a VkCommandPool.
            // The pool is owned by the Queue; CommandAllocator maps to a
            // per-frame pool reset cycle.
            VkCommandPool vk_command_pool = VK_NULL_HANDLE;
        };
    }
}
