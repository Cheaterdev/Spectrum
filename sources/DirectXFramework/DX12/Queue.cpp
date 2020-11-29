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


		auto name = std::string("DXQueue: ") + std::string(magic_enum::enum_name(type));

		native->SetName(convert(name).c_str());

		queue_thread = std::thread([this, name]
			{
				SetThreadName(name);

				for (;;)
				{
					PROFILE(L"Wait");

					std::function<void()> task;
					{
					
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this]
							{
								return this->stop || !this->tasks.empty();
							});

						if (this->stop && this->tasks.empty())
							return;


						task = std::move(this->tasks.front());
					
						this->tasks.pop();
					}	
					
					PROFILE(L"Task");

					task();
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
		signal_and_wait();
		stop = true;
		condition.notify_all();

		if (queue_thread.joinable())
			queue_thread.join();
	}

	Queue::~Queue()
	{
		stop_all();
	}

	FenceWaiter Queue::signal()
	{
		const UINT64 fence = ++m_fenceValue;
		native->Signal(commandListCounter.m_fence.Get(), fence);
		return { &commandListCounter,fence };
	}

	void  Queue::gpu_wait(FenceWaiter waiter)
	{
		native->Wait(waiter.fence->m_fence.Get(), waiter.value);
	}

	FenceWaiter Queue::signal(Fence& fence, UINT64 value)
	{
		native->Signal(fence.m_fence.Get(), value);

		return { &fence, value };
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

	void Queue::signal_and_wait()
	{
		std::promise<int> promise;
		auto result = promise.get_future();

		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			FenceWaiter waiter = signal();

			tasks.emplace([&promise, &waiter]() {
				waiter.wait();
				promise.set_value(0);
				});
			condition.notify_one();
		}

		result.wait();
	}

	bool Queue::is_complete(UINT64 fence)
	{
		return commandListCounter.get_completed_value() >= fence;
	}

	ComPtr<ID3D12CommandQueue> Queue::get_native()
	{
		return native;
	}


	void Queue::update_tile_mappings(ID3D12Resource* pResource, UINT NumResourceRegions, const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates, const D3D12_TILE_REGION_SIZE* pResourceRegionSizes, ID3D12Heap* pHeap, UINT NumRanges, const D3D12_TILE_RANGE_FLAGS* pRangeFlags, const UINT* pHeapRangeStartOffsets, const UINT* pRangeTileCounts, D3D12_TILE_MAPPING_FLAGS Flags)
	{
		native->UpdateTileMappings(pResource, NumResourceRegions, pResourceRegionStartCoordinates, pResourceRegionSizes, pHeap, NumRanges, pRangeFlags, pHeapRangeStartOffsets, pRangeTileCounts, Flags);
	}

	// synchronized
	FenceWaiter Queue::execute_internal(CommandList* list)
	{

		bool need_wait = has_tasks();


		SPECTRUM_TRY


			if (stop)
				return { &commandListCounter,m_fenceValue };

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

				if (type == CommandListType::COMPUTE)
				{
					auto direct_queue = Device::get().get_queue(CommandListType::DIRECT)->get_native();

					direct_queue->ExecuteCommandLists(1, &s[0]);
					auto waiter = Device::get().get_queue(CommandListType::DIRECT)->signal();
					gpu_wait(waiter);
				//	native->Wait(waiter.fence->m_fence.Get(), waiter.value);
				}
				else
				{

					native->ExecuteCommandLists(1, &s[0]);
				}

				native->ExecuteCommandLists(1, &s[1]);
			}
			else
			{
				ID3D12CommandList* s = list->get_native_list().Get();
				native->ExecuteCommandLists(1, &s);
			}
		}

		last_known_fence = signal().value;
		list->execute_fence.set_value({&commandListCounter, last_known_fence});

		auto cl = list->get_ptr();

		tasks.emplace([cl, this]() {
			auto list = cl;

			FenceWaiter fence = list->get_execution_fence().get();
			assert(&commandListCounter == fence.fence);

			fence.wait();

			SPECTRUM_TRY
				list->on_execute();
			SPECTRUM_CATCH
			});


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
			commandListCounter.wait(last_known_fence);
			process_tasks();
		}

		return { &commandListCounter,last_known_fence };
	}


	std::shared_future<FenceWaiter> Queue::execute(std::shared_ptr<CommandList> list)
	{
		assert(this->type == list->type);
		std::unique_lock<std::mutex> lock(queue_mutex);

		if (!list->wait_for_execution_count)
		{
			std::promise<FenceWaiter> res;
			res.set_value(execute_internal(list.get()));
			return res.get_future();
		}

		return list->get_execution_fence();
	}


	std::shared_future<FenceWaiter> Queue::execute(CommandList* list)
	{
		return execute(list->get_ptr());
	}


}
