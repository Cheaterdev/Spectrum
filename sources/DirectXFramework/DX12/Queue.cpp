#include "pch.h"
#include "Samplers.h"
#include "Device12.h"

namespace DX12
{

	Queue::Queue(CommandListType type, Device* device) : commandListCounter(*device)
	{
		auto t = CounterManager::get().start_count<Queue>();
		this->type = type;
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = static_cast<D3D12_COMMAND_LIST_TYPE>(type);
		queueDesc.NodeMask = 1;
		device->get_native_device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&native));

		m_fenceValue = 0;

		static int counter = 0;
		name = std::wstring(L"Queue_") + std::to_wstring(++counter);
		native->SetName(name.c_str());
		queue_thread = std::thread([this]
			{
				std::string name;

				if (this->type == CommandListType::DIRECT)
					name = "DIRECT";
				else if (this->type == CommandListType::COPY)
					name = "COPY";
				else if (this->type == CommandListType::COMPUTE)
					name = "COMPUTE";
				SetThreadName(std::string("DXQueue: ") + name);

				for (;;)
				{
					CommandList::ptr list;
					UINT64 fence;
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this]
							{
								return this->stop || !this->tasks.empty();
							});

						if (this->stop && this->tasks.empty())
							return;

						// TODO:: get all tasks, not only 1!!!!
						list = std::move(this->tasks.front());
						fence = list->get_execution_fence().get();
						this->tasks.pop();
					}
					commandListCounter.wait_for(commandListEvent, fence);

					SPECTRUM_TRY
						list->on_execute();
					SPECTRUM_CATCH

				}
			});

		del_func = [this](CommandList* list)
		{
			if (stop)
				delete list;
			else
			{
				std::lock_guard<std::mutex> g(list_mutex);
				lists.emplace(list, del_func);
			}
		};
	}

	void Queue::stop_all()
	{
		wait();
		stop = true;
		condition.notify_all();

		if (queue_thread.joinable())
			queue_thread.join();
	}

	Queue::~Queue()
	{
		stop_all();
	}

	UINT64 Queue::signal()
	{
		const UINT64 fence = ++m_fenceValue;
		native->Signal(commandListCounter.m_fence.Get(), fence);
		return fence;
	}

	FenceWaiter Queue::signal(Fence& fence, UINT64 value)
	{
		native->Signal(fence.m_fence.Get(), value);

		return { fence, value };
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

		std::shared_ptr<CommandList> res_ptr;
		res_ptr.reset(new CommandList(type), del_func);
		return res_ptr;
	}

	void Queue::wait(UINT64 value)
	{
		static thread_local Event e;
		commandListCounter.wait_for(e, value);
	}

	void Queue::wait()
	{
		wait(m_fenceValue);
	}

	bool Queue::is_complete(UINT64 fence)
	{
		return commandListCounter.get_completed_value() >= fence;
	}

	UINT64 Queue::get_last_step()
	{
		return m_fenceValue;
	}

	ComPtr<ID3D12CommandQueue> Queue::get_native()
	{
		return native;
	}


	// synchronized
	UINT64 Queue::execute_internal(CommandList* list)
	{

		bool need_wait = has_tasks();


		SPECTRUM_TRY


			if (stop)
				return m_fenceValue;

		for(auto &w:list->waits)
		{
//			native->Wait(w.fence.Get(), w.value);
		}


		for (auto& fun : list->on_send_funcs)
		{
			fun();
		}
		list->on_send_funcs.clear();

		//	fix_transitions
		{

			auto transition_list = list->fix_pretransitions();

			if (transition_list)
			{
				ID3D12CommandList* s[2] = { transition_list->get_native().Get(), list->get_native_list().Get() };


				//native->ExecuteCommandLists(2, s);


				native->ExecuteCommandLists(1, &s[0]);
				native->ExecuteCommandLists(1, &s[1]);
			}
			else
			{
				ID3D12CommandList* s = list->get_native_list().Get();
				native->ExecuteCommandLists(1, &s);
			}
		}

		last_known_fence = signal();
		list->execute_fence.set_value(last_known_fence);

		tasks.emplace(list->get_ptr());


		list->on_send();

		SPECTRUM_CATCH

			condition.notify_one();


		if (list->parent_list)
		{
			if (list->parent_list->child_executed())
			{
				execute_internal(static_cast<CommandList*>(list->parent_list.get()));
			}

		}

		if (need_wait)
		{
			wait(last_known_fence);
			process_tasks();
		}

		return last_known_fence;
	}


	std::shared_future<UINT64> Queue::execute(std::shared_ptr<CommandList> list)
	{
		assert(this->type == list->type);
		std::unique_lock<std::mutex> lock(queue_mutex);

		if (!list->wait_for_execution_count)
		{
			std::promise<UINT64> res;
			res.set_value(execute_internal(list.get()));
			return res.get_future();
		}

		return list->get_execution_fence();
	}


	std::shared_future<UINT64> Queue::execute(CommandList* list)
	{
		return execute(list->get_ptr());
	}


}
