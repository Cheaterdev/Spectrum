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
	class Queue: public API::Queue	, public TypedObject<Queue>
	{
		friend class API::Queue;
		Device& device;
		std::wstring name;

		HAL::Fence commandListCounter;

	//	HAL::Fence internalCounter;
	//	UINT64 m_InternalCounterValue;

		UINT64 m_fenceValue;

		SingleThreadExecutor gpu_wait_thread;
		SingleThreadExecutor gpu_execute_thread;
	
		std::function<void(CommandList*)> del_func;
		std::function<void(TransitionCommandList*)> del_transition;

		std::queue<std::shared_ptr<TransitionCommandList>> transition_lists;

		std::queue<std::shared_ptr<CommandList>> lists;
		std::mutex list_mutex;

		std::mutex queue_mutex;
		std::mutex states_mutex;


			std::mutex submit_mutex;


		bool stop = false;

		HAL::CommandListType type;

		void execute_internal(UINT64 fence_value, std::vector<CommandList::ptr> lists);
		uint64 frequency;

	public:
		void update_tile_mappings(const HAL::update_tiling_info& infos);

	public:
		Queue(HAL::CommandListType type, Device& device);
		~Queue();

		std::shared_ptr<CommandList> get_free_list();
		std::shared_ptr<TransitionCommandList> get_transition_list();
		using ptr = std::shared_ptr<Queue>;
		UINT64 get_frequency() const;
		ClockCalibrationInfo get_clock_time() const;

		void stop_all();
  		bool is_complete(UINT64 fence);

		void signal_and_wait();
  		HAL::FenceWaiter signal();
		// Submit a group. There is deliberately no overload taking a bare
		// array of lists: barriers are computed across a whole group (see
		// CommandListGroup::compile_transitions), so a batch that never
		// became a group would reach the GPU with its barriers unfilled.
		HAL::FenceWaiter execute(const CommandListGroup& group);
		HAL::FenceWaiter signal(HAL::Fence& fence, UINT64 value);
		void gpu_wait(HAL::FenceWaiter waiter);

		void run(std::function<void()> f);
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
			Device& device;
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