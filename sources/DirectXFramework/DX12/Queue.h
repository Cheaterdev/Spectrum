#pragma once

namespace DX12
{

	class TransitionCommandList;
	class Device;
	class Queue
	{
		std::wstring name;
		ComPtr<ID3D12CommandQueue> native;
		
		Fence commandListCounter;
		Event commandListEvent;

		UINT64 m_fenceValue;

		SingleThreadExecutor executor;
	//	std::thread  queue_thread;;
		std::function<void(CommandList*)> del_func;
		std::function<void(TransitionCommandList*)> del_transition;

		std::queue<std::shared_ptr<TransitionCommandList>> transition_lists;

		std::queue<std::shared_ptr<CommandList>> lists;
		std::mutex list_mutex;
	//	std::queue<std::function<void()>> tasks;

		std::mutex queue_mutex;
		std::mutex states_mutex;
		//std::condition_variable condition;

		bool stop = false;
	
		CommandListType type;

		FenceWaiter last_executed_fence;

		FenceWaiter execute_internal(CommandList* list);
	//	std::mutex tasks_mutex;


	//	std::thread  send_queue_thread;
	//	std::condition_variable send_condition;
	//	std::queue<std::function<void()>> send_tasks;
	///	std::mutex send_queue_mutex;
		FenceWaiter signal_internal();
		void gpu_wait_internal(FenceWaiter waiter);

		FenceWaiter run_transition_list(FenceWaiter after, std::shared_ptr<TransitionCommandList>& list);
	public:
		void update_tile_mappings(const update_tiling_info &infos);

	public:
		Queue(CommandListType type, Device* device);
		~Queue();

		std::shared_ptr<CommandList> get_free_list();
		std::shared_ptr<TransitionCommandList> get_transition_list();
		using ptr = std::shared_ptr<Queue>;

		ComPtr<ID3D12CommandQueue> get_native();



		void stop_all();
		FenceWaiter signal();
		void signal_and_wait();
		bool is_complete(UINT64 fence);

		FenceWaiter execute(std::shared_ptr<CommandList> list);
		FenceWaiter execute(CommandList* list);

		FenceWaiter signal(Fence& fence, UINT64 value);

		void gpu_wait(FenceWaiter waiter);
	};
	
}