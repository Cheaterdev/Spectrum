export module HAL:Queue;

import Scheduler;
import Executors;
import :Fence;
import :Types;
import :TiledMemoryManager;
import :API.Queue;

export namespace HAL
{
	class Queue: public API::Queue
	{
		friend class API::Queue;
		std::wstring name;

		HAL::Fence commandListCounter;
		HAL::Event commandListEvent;

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

		HAL::FenceWaiter run_transition_list(HAL::FenceWaiter after,const std::shared_ptr<TransitionCommandList>& list);
	public:
		void update_tile_mappings(const HAL::update_tiling_info& infos);

	public:
		Queue(HAL::CommandListType type, Device* device);
		~Queue();

		std::shared_ptr<CommandList> get_free_list();
		std::shared_ptr<TransitionCommandList> get_transition_list();
		using ptr = std::shared_ptr<Queue>;


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


}