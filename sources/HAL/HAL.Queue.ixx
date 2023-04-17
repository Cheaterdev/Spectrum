export module HAL:Queue;
import Core;
import :Fence;
import :Types;
import :TiledMemoryManager;
import :API.Queue;

export namespace HAL
{
	struct ClockCalibrationInfo
	{
		uint64 cpu_time;
		uint64 gpu_time;
		uint64 frequency;
	};
	class Queue: public API::Queue
	{
		friend class API::Queue;
		std::wstring name;

		HAL::Fence commandListCounter;

		UINT64 m_fenceValue;

		SingleThreadExecutor executor;
		std::function<void(CommandList*)> del_func;
		std::function<void(TransitionCommandList*)> del_transition;

		std::queue<std::shared_ptr<TransitionCommandList>> transition_lists;

		std::queue<std::shared_ptr<CommandList>> lists;
		std::mutex list_mutex;

		std::mutex queue_mutex;
		std::mutex states_mutex;

		bool stop = false;

		HAL::CommandListType type;

		HAL::FenceWaiter last_executed_fence;

		FenceWaiter signal_internal();
		HAL::FenceWaiter execute_internal(CommandList* list);
		uint64 frequency;

		HAL::FenceWaiter run_transition_list(HAL::FenceWaiter after, TransitionCommandList* list);

	public:
		void update_tile_mappings(const HAL::update_tiling_info& infos);

	public:
		Queue(HAL::CommandListType type, Device* device);
		~Queue();

		std::shared_ptr<CommandList> get_free_list();
		std::shared_ptr<TransitionCommandList> get_transition_list();
		using ptr = std::shared_ptr<Queue>;
		inline UINT64 get_frequency() const {
			return frequency;
				
		}
		ClockCalibrationInfo get_clock_time() const;
		HAL::FenceWaiter get_last_fence()
		{
			return last_executed_fence;
		}
		void stop_all();
		HAL::FenceWaiter signal();
		void signal_and_wait();
		bool is_complete(UINT64 fence);

		HAL::FenceWaiter execute(std::shared_ptr<CommandList> list);
		HAL::FenceWaiter execute(CommandList* list);

		HAL::FenceWaiter signal(HAL::Fence& fence, UINT64 value);

		void gpu_wait(HAL::FenceWaiter waiter);
	};

	struct StorageRequest
	{
		Resource::ptr resource;
		std::filesystem::path file;
		uint64 file_offset;
		uint64 size;
		uint64 uncompressed_size;
		bool compressed;

		struct Buffer
		{
			uint64 offset;
		};

		struct Texture
		{
			uint subresource;
			uint count;
		};

		std::variant<Buffer,Texture> operation;
	};
	class DirectStorageQueue:public API::DirectStorageQueue

	{
			std::mutex queue_mutex;
		UINT64 m_fenceValue = 0;
			HAL::Fence requestCounter;

		SingleThreadExecutor executor;
		public:
			using ptr = std::shared_ptr<DirectStorageQueue>;
		
		DirectStorageQueue(Device& device);
		~DirectStorageQueue();


		void stop_all();
		HAL::FenceWaiter signal();
		void signal_and_wait();
		bool is_complete(UINT64 fence);


		HAL::FenceWaiter execute(StorageRequest request);
		void flush();
		FenceWaiter get_waiter();
	};
}