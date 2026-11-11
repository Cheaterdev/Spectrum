export module HAL:API.Queue;
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
            std::vector<ID3D12CommandList*> queued;
        protected:
            D3D::CommandQueue native;

            void execute(const API::CommandList* list);
            void flush();
            void signal(Fence& fence, Fence::CounterType value);
            void gpu_wait(HAL::FenceWaiter waiter);

            void construct(HAL::CommandListType type, Device* device);
        public:
            virtual ~Queue() = default;

            D3D::CommandQueue get_native();
        };

        class DirectStorageQueue
        {
        protected:
            D3D::StorageQueue native;
            D3D::StorageFactory factory;

            // Opened on the first request that reads them, by normalized absolute path. Guarded by
            // HAL::DirectStorageQueue::queue_mutex.
            std::unordered_map<std::wstring, D3D::StorageFile> files;

            // The open file for `path`, opened now if it is not yet. queue_mutex held.
            D3D::StorageFile cached_file(const std::filesystem::path& path);
        public:
            virtual ~DirectStorageQueue() = default;

            D3D::StorageQueue get_native();
        };
    }

}