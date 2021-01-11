#pragma once
#include "Fence.h"
#include "States.h"

namespace DX12
{

	class TransitionCommandList;
	class Device;
	class Queue: public Events::Runner //do not use runner, its obnly for testing here
	{
		std::wstring name;
		ComPtr<ID3D12CommandQueue> native;
		
		Fence commandListCounter;
		Event commandListEvent;

		UINT64 m_fenceValue;
		std::thread  queue_thread;;
		std::function<void(CommandList*)> del_func;
		std::function<void(TransitionCommandList*)> del_transition;

		std::queue<std::shared_ptr<TransitionCommandList>> transition_lists;

		std::queue<std::shared_ptr<CommandList>> lists;
		std::mutex list_mutex;
		std::queue<std::function<void()>> tasks;

		std::mutex queue_mutex;
		std::condition_variable condition;

		bool stop = false;
	
		CommandListType type;

		UINT64 last_known_fence = 0;
		FenceWaiter execute_internal(CommandList* list);
	
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

		std::shared_future<FenceWaiter> execute(std::shared_ptr<CommandList> list);
		std::shared_future<FenceWaiter> execute(CommandList* list);

		FenceWaiter signal(Fence& fence, UINT64 value);

		void gpu_wait(FenceWaiter waiter);
	};
	
}