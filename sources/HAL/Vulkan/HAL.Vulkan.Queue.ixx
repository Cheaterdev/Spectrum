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
            uint32_t      family_idx = std::numeric_limits<uint32_t>::max();

            void execute(const API::CommandList* list);
            void flush();
            void signal(Fence& fence, Fence::CounterType value);
            void gpu_wait(HAL::FenceWaiter waiter);
            void construct(HAL::CommandListType type, Device* device);

        public:
            virtual ~Queue() = default;

            VkQueue get_native() const;
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
