module Graphics;
import :Device;
import :GPUTimer;
import :CommandList;

import Debug;

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

		std::shared_ptr<Graphics::CommandList> res_ptr;
		res_ptr.reset(new Graphics::CommandList(type), del_func);
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

		std::shared_ptr<Graphics::TransitionCommandList> res_ptr;
		res_ptr.reset(new Graphics::TransitionCommandList(type), del_transition);
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

	FenceWaiter Queue::run_transition_list(FenceWaiter after, const std::shared_ptr<TransitionCommandList>& list)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		API::Queue::gpu_wait(after);
		API::Queue::execute(list.get());
		return signal_internal();
	}

	FenceWaiter Queue::execute_internal(CommandList* list)
	{
		PROFILE(L"execute_internal");

		if (stop || !Graphics::Device::get().alive)
			return { &commandListCounter,m_fenceValue };

		std::unique_lock<std::mutex> lock(queue_mutex);


		auto cl = to_hal(list)->get_ptr();

		{
			PROFILE(L"update_tile_mappings");

			bool has_updates = false;

			auto& updates = to_hal(list)->tile_updates;
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
						auto& updates = to_hal(list)->tile_updates;
						for (auto& u : updates)
							(u.resource)->get_tiled_manager().on_tile_update(u);
						updates.clear();
					});
			}
		}


		auto transition_list = to_hal(list)->fix_pretransitions();

		if (transition_list)
		{
			if (transition_list->get_type() == to_hal(list)->get_type())
			{
				API::Queue::execute(transition_list.get());
				API::Queue::execute(list);
			}
			else
			{
				// Need to request other queue to make a proper transition.
				// It's OK, but better to avoid this
				auto queue = Graphics::Device::get().get_queue(transition_list->get_type());
				auto waiter = queue->run_transition_list(last_executed_fence, std::static_pointer_cast<HAL::TransitionCommandList>(transition_list));

				API::Queue::gpu_wait(waiter);
				API::Queue::execute(list);
			}

		}
		else
		{
			API::Queue::execute(list);
		}
		const FenceWaiter execution_fence = signal_internal();

		{
			PROFILE(L"on_send");
			to_hal(list)->on_send(execution_fence);
			to_hal(list)->free_resources();
		}

		executor.enqueue([cl, this]()
			{
				if (!Graphics::Device::get().alive) return;

				auto list = cl;
				to_hal(list)->get_execution_fence().get().wait();
				PROFILE(L"on_execute");
				to_hal(list)->on_execute();
			});

		last_executed_fence = execution_fence;
		return execution_fence;
	}

	FenceWaiter Queue::execute(std::shared_ptr<CommandList> list)
	{
		assert(this->type == to_hal(list)->type);
		return execute_internal (list.get());
	}

	FenceWaiter Queue::execute(CommandList* list)
	{
		return execute(to_hal(list)->get_ptr());
	}
}