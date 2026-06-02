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
            // CommandList allocates VkCommandBuffer from this pool.
            friend class CommandList;

        protected:
            VkCommandPool vk_command_pool = VK_NULL_HANDLE;

        public:
            virtual ~CommandAllocator() = default;
        };
    }
}
