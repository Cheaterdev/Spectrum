module HAL:Queue;
import <HAL.h>;

import Core;

import HAL;
#undef THIS

using namespace HAL;
namespace HAL
{
	void Queue::stop_all()
	{
		signal_and_wait();
		stop = true;

		lists = {};
	}

	Queue::~Queue()
	{
		stop_all();
	}



/*	FenceWaiter Queue::signal_internal()
	{
		const UINT64 fence = ++m_fenceValue;
		 API::Queue::signal(commandListCounter, fence);

		 return FenceWaiter{ &commandListCounter, fence, type};
	}
		 */

	std::shared_ptr<CommandList> Queue::get_free_list()
	{
		std::lock_guard<std::mutex> g(list_mutex);

		if (!lists.empty())
		{
			auto e = lists.front();
			lists.pop();
			return e;
		}

		std::shared_ptr<HAL::CommandList> res_ptr;
		res_ptr.reset(new HAL::CommandList(type), del_func);
		return res_ptr;
	}

	std::shared_ptr<TransitionCommandList> Queue::get_transition_list()
	{
		std::lock_guard<std::mutex> g(list_mutex);

		if (!transition_lists.empty())
		{
			auto e = transition_lists.front();
			transition_lists.pop();
			return e;
		}

		std::shared_ptr<TransitionCommandList> res_ptr;
		res_ptr.reset(new TransitionCommandList(type), del_transition);
		return res_ptr;
	}


	bool Queue::is_complete(UINT64 fence)
	{
		return commandListCounter.get_completed_value() >= fence;
	}

	/*FenceWaiter Queue::run_transition_list(FenceWaiter after, TransitionCommandList* list)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		API::Queue::gpu_wait(after);
		API::Queue::execute(&list->get_compiled());
		return signal_internal();
	}	 */

	void Queue::execute_internal(UINT64 fence_value, std::list<CommandList::ptr> lists)
	{
				std::unique_lock<std::mutex> lock(queue_mutex);

		PROFILE(L"execute_internal");
		{
			PROFILE(L"update_tile_mappings");

			bool has_updates = false;

			/// In packed lists make with 1 go
			for (auto& list : lists)
			{
				auto& updates = (list)->tile_updates;
				{
					for (auto& u : updates)
					{
						has_updates = true;
						update_tile_mappings(u);
					}
				}
			}
			

		/*	if (has_updates)
			{

				API::Queue::signal(commandListCounter, fence_value);

				FenceWaiter execution_fence{ &commandListCounter, fence_value, type};


	//			FenceWaiter execution_fence = signal_internal();

				gpu_wait_thread.enqueue([lists, this, execution_fence]()
					{
						PROFILE(L"update_tile_mappings result");
						execution_fence.wait();
						for (auto& list : lists)
						{
						
						}
					});
			}		   */
		}
		 	
		for (auto& list : lists)
		{
			API::Queue::gpu_wait(list->dstorage_fence);

			if (list->dstorage_fence)
				Device::get().get_ds_queue().flush();
		}

		std::list<TransitionCommandList::ptr> transition_lists;
			for (auto& list : lists)
			{

#ifdef PRETRANSITIONS_FIX
				auto transition_list = (list)->fix_pretransitions();
				  assert(!transition_list);
				if (transition_list)
				{
					transition_lists.emplace_back(transition_list);
					PROFILE(L"execute_transitions");
					if (transition_list->get_type() == (list)->get_type())
					{
						API::Queue::execute(&transition_list->get_compiled());
						API::Queue::execute(&list->compiler.get_list());
					}
					else
					{
						// Need to request other queue to make a proper transition.
						// It's OK, but better to avoid this
						auto queue = HAL::Device::get().get_queue(transition_list->get_type());
						auto waiter = queue->run_transition_list(last_executed_fence, transition_list.get());

						API::Queue::gpu_wait(waiter);
						API::Queue::execute(&list->compiler.get_list());
					}

				}
				else		  
#endif
				{
					PROFILE(L"execute_simple");
					API::Queue::execute(&list->compiler.get_list());	 
				}

			
			
			}
		
		API::Queue::signal(commandListCounter, fence_value);

		 FenceWaiter execution_fence{ &commandListCounter, fence_value, type};

		  gpu_wait_thread.enqueue([lists, transition_lists, execution_fence, this]()
			{
				if (!HAL::Device::get().alive) return;

				PROFILE(L"on_execute wait");
				execution_fence.wait();

			


				PROFILE(L"on_execute process");
				for (auto& transition_list : transition_lists)
					transition_list->on_execute();

				for (auto& list : lists)
				{
					auto& updates = (list)->tile_updates;
				for (auto& u : updates)
					(u.resource)->get_tiled_manager().on_tile_update(u);
				updates.clear();

					(list)->on_execute();
				}
					
			});

	
	}
			

   
	void  Queue::gpu_wait(FenceWaiter waiter)
	{
		if (!waiter) return;

					std::unique_lock<std::mutex> lock(submit_mutex);
			gpu_execute_thread.enqueue([this, waiter]() {
			std::unique_lock<std::mutex> lock(queue_mutex);
		API::Queue::gpu_wait(waiter);
			});


		
	}
	  
	FenceWaiter Queue::signal(Fence& fence, UINT64 value)
	{
		assert(false);
		std::unique_lock<std::mutex> lock(submit_mutex);
		gpu_execute_thread.enqueue([this, fence, value]() mutable{
			std::unique_lock<std::mutex> lock(queue_mutex);
			API::Queue::signal(fence, value);
			});

		return FenceWaiter{ &fence, value , type };
	}


 	FenceWaiter Queue::signal()
	{
		std::unique_lock<std::mutex> lock(submit_mutex);

		const UINT64 value = ++m_fenceValue;
		gpu_execute_thread.enqueue([this, value]() {
			std::unique_lock<std::mutex> lock(queue_mutex);
			API::Queue::signal(commandListCounter, value);
			});

		return FenceWaiter{ &commandListCounter, value , type };
		
	}


	void Queue::signal_and_wait()
	{
		PROFILE(L"signal_and_wait");
		std::unique_lock<std::mutex> lock(submit_mutex);

		const UINT64 value = ++m_fenceValue;

		//wait for GPU
		gpu_execute_thread.enqueue([this, value]() {
			std::unique_lock<std::mutex> lock(queue_mutex);
			API::Queue::signal(commandListCounter, value);
			});
		FenceWaiter waiter{ &commandListCounter, value , type };

		waiter.wait();

		std::promise<int> promise;
		auto result = promise.get_future();
		// wait all execute commands done
		gpu_wait_thread.enqueue([&promise]() {

			promise.set_value(0);
			});


		result.wait();
	}

	HAL::FenceWaiter Queue::execute(std::list<CommandList::ptr> list)
	{	
		std::unique_lock<std::mutex> lock(submit_mutex);

		const UINT64 fence = ++m_fenceValue;
		gpu_execute_thread.enqueue([this, list, fence]() {
			execute_internal(fence, list);
			});

		return  HAL::FenceWaiter(&commandListCounter, fence, type);
	}
	   

		void Queue::run(std::function<void()> f)
		{				
			std::unique_lock<std::mutex> lock(submit_mutex);
			gpu_execute_thread.enqueue(f);
		}
}