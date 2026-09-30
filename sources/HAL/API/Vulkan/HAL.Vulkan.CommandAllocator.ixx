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
            VkCommandPool      vk_command_pool = VK_NULL_HANDLE;
            // VkCommandPool is not thread-safe: all vkCmd* calls and pool operations
            // (allocate / reset) on any buffer from this pool must be serialized.
            mutable std::mutex pool_mutex;

            // Command buffers allocated from vk_command_pool, handed out in order and
            // reused after reset(): vkResetCommandPool keeps buffers allocated, so
            // allocating per begin() grows the pool forever. Caller holds pool_mutex.
            std::vector<VkCommandBuffer> buffers;
            size_t                       next_buffer = 0;
            VkCommandBuffer acquire(VkDevice vk_dev);

        public:
            virtual ~CommandAllocator() = default;
        };
    }
}
