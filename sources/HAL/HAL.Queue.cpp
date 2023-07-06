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
	}

	Queue::~Queue()
	{
		stop_all();
	}

	FenceWaiter Queue::signal()
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		return signal_internal();
	}

	void  Queue::gpu_wait(FenceWaiter waiter)
	{
		if (!waiter) return;

		std::unique_lock<std::mutex> lock(queue_mutex);
		API::Queue::gpu_wait(waiter);
	}

	FenceWaiter Queue::signal_internal()
	{
		const UINT64 fence = ++m_fenceValue;
		 API::Queue::signal(commandListCounter, fence);

		 return FenceWaiter{ &commandListCounter, fence };
	}

	FenceWaiter Queue::signal(Fence& fence, UINT64 value)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		API::Queue::signal(fence, value);
		return FenceWaiter{ &fence, value };
	}

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

	void Queue::signal_and_wait()
	{
		std::promise<int> promise;
		auto result = promise.get_future();

		{
			FenceWaiter waiter = signal();

			executor.enqueue([&promise, waiter]() {
				waiter.wait();
				promise.set_value(0);
				});
		}

		result.wait();
	}

	bool Queue::is_complete(UINT64 fence)
	{
		return commandListCounter.get_completed_value() >= fence;
	}

	FenceWaiter Queue::run_transition_list(FenceWaiter after, TransitionCommandList* list)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		API::Queue::gpu_wait(after);
		API::Queue::execute(&list->get_compiled());
		return signal_internal();
	}

	FenceWaiter Queue::execute_internal(CommandList* list)
	{
		PROFILE(L"execute_internal");

		if (stop || !HAL::Device::get().alive)
			return { &commandListCounter,m_fenceValue };

		std::unique_lock<std::mutex> lock(queue_mutex);


		auto cl = (list)->get_ptr();

		{
			PROFILE(L"update_tile_mappings");

			bool has_updates = false;

			auto& updates = (list)->tile_updates;
			{
				for (auto& u : updates)
				{
					has_updates = true;
					update_tile_mappings(u);
				}
			}

			if (has_updates)
			{
				FenceWaiter execution_fence = signal_internal();

				executor.enqueue([cl, this, execution_fence]()
					{
						execution_fence.wait();
						auto list = cl;
						auto& updates = (list)->tile_updates;
						for (auto& u : updates)
							(u.resource)->get_tiled_manager().on_tile_update(u);
						updates.clear();
					});
			}
		}

		API::Queue::gpu_wait(list->dstorage_fence);

		if(list->dstorage_fence)
			Device::get().get_ds_queue().flush();

		auto transition_list = (list)->fix_pretransitions();

		if (transition_list)
		{
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
		{
				PROFILE(L"execute_simple");
				API::Queue::execute(&list->compiler.get_list());
		}
	//	Log::get() << Log::LEVEL_ERROR<< "Send " << list->name << Log::endl;

		const FenceWaiter execution_fence = signal_internal();

		//{
		//	PROFILE(L"on_send");
		//	(list)->on_send(execution_fence);
		//	(list)->free_resources();
		//}

		executor.enqueue([cl,transition_list, execution_fence, this]()
			{
				if (!HAL::Device::get().alive) return;

				auto list = cl;
				execution_fence.wait();
				PROFILE(L"on_execute");
				if(transition_list) transition_list->on_execute();
				(list)->on_execute();
			});

		last_executed_fence = execution_fence;

		//execution_fence.wait();
		return execution_fence;
	}

	FenceWaiter Queue::execute(std::shared_ptr<CommandList> list)
	{
		assert(this->type == (list)->type);
		return execute_internal (list.get());
	}

	FenceWaiter Queue::execute(CommandList* list)
	{
		return execute((list)->get_ptr());
	}
}