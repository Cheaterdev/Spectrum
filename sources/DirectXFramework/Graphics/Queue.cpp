module Graphics:Queue;
import :Samplers;
import :Device;
import :GPUTimer;

import Debug;

import HAL;

using namespace HAL;
namespace Graphics
{

	Queue::Queue(CommandListType type, Device* device) : commandListCounter(*device)
	{
		auto t = CounterManager::get().start_count<Queue>();
		this->type = type;
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};

		if (type == CommandListType::DIRECT)
		{
			queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
		}
		else
		{
			queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		}


		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
		queueDesc.Type = to_native(type);
		queueDesc.NodeMask = 1;
		device->get_native_device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&native));

		m_fenceValue = 0;


		auto name = std::string("DXQueue: ") + std::string(magic_enum::enum_name(type));

		native->SetName(convert(name).c_str());


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

		del_transition = [this](TransitionCommandList* list)
		{
			if (stop)
				delete list;
			else
			{
				std::lock_guard<std::mutex> g(list_mutex);
				transition_lists.emplace(list, del_transition);
			}
		};


	}

	void Queue::stop_all()
	{
		signal_and_wait();
		stop = true;


	}

	Queue::~Queue()
	{
		stop_all();
	}

	FenceWaiter Queue::signal_internal()
	{

		const UINT64 fence = ++m_fenceValue;
		native->Signal(commandListCounter.m_fence.Get(), fence);
		return { &commandListCounter,fence };
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
		gpu_wait_internal(waiter);
	}
	void  Queue::gpu_wait_internal(FenceWaiter waiter)
	{
		if (!waiter) return;
		native->Wait(waiter.fence->m_fence.Get(), waiter.value);
	}

	FenceWaiter Queue::signal(Fence& fence, UINT64 value)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
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

	void Queue::signal_and_wait_internal()
	{
		std::promise<int> promise;
		auto result = promise.get_future();
		{
			FenceWaiter waiter = signal_internal();

			executor.enqueue([&promise, waiter]() {
				waiter.wait();
				promise.set_value(0);
				});
		}

		result.wait();

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

	ComPtr<ID3D12CommandQueue> Queue::get_native()
	{
		return native;
	}


	void Queue::update_tile_mappings(const update_tiling_info& infos)
	{
		for (auto& [heap, tiles] : infos.tiles)
		{
			std::vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates;
			std::vector<D3D12_TILE_REGION_SIZE> regionSizes;
			std::vector<D3D12_TILE_RANGE_FLAGS> rangeFlags;
			std::vector<UINT> heapRangeStartOffsets;
			std::vector<UINT> rangeTileCounts;

			for (const ResourceTile& tile : tiles)
			{
				D3D12_TILED_RESOURCE_COORDINATE TRC;
				TRC.X = tile.pos.x;
				TRC.Y = tile.pos.y;
				TRC.Z = tile.pos.z;
				TRC.Subresource = tile.subresource;

				D3D12_TILE_REGION_SIZE TRS;
				TRS.UseBox = false;
				TRS.Width = 1;
				TRS.Height = 1;
				TRS.Depth = 1;
				TRS.NumTiles = tile.heap_position.count;

				startCoordinates.push_back(TRC);
				regionSizes.push_back(TRS);

				if (tile.heap_position.heap)
					rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_NONE);
				else
					rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_NULL);

				heapRangeStartOffsets.push_back((UINT)tile.heap_position.offset);
				rangeTileCounts.push_back((UINT)TRS.NumTiles);
			}

			native->UpdateTileMappings(
				infos.resource->get_dx(),
				UINT(startCoordinates.size()),
				&startCoordinates[0],
				&regionSizes[0],
				heap ? heap->native_heap.Get() : nullptr,
				UINT(rangeFlags.size()),
				&rangeFlags[0],
				&heapRangeStartOffsets[0],
				&rangeTileCounts[0],
				D3D12_TILE_MAPPING_FLAG_NONE
			);
		}

		if (infos.source)
		{

			D3D12_TILED_RESOURCE_COORDINATE target;
			target.X = infos.pos.x;
			target.Y = infos.pos.y;
			target.Z = infos.pos.z;
			target.Subresource = infos.target_subres;


			D3D12_TILED_RESOURCE_COORDINATE source;
			source.X = infos.source_pos.x;
			source.Y = infos.source_pos.y;
			source.Z = infos.source_pos.z;
			source.Subresource = infos.source_subres;

			D3D12_TILE_REGION_SIZE TRS;
			TRS.UseBox = true;
			TRS.Width = infos.size.x;
			TRS.Height = infos.size.y;
			TRS.Depth = infos.size.z;
			TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;

			auto dx_target_resource = infos.resource->get_dx();
			auto dx_source_resource = infos.source->get_dx();
			native->CopyTileMappings(dx_target_resource, &target, dx_source_resource, &source, &TRS, D3D12_TILE_MAPPING_FLAG_NONE);
		}
		//	signal_and_wait_internal();

	}
	FenceWaiter Queue::run_transition_list(FenceWaiter after, std::shared_ptr<TransitionCommandList>& list)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		gpu_wait_internal(after);

		ID3D12CommandList* s[] = { list->get_native().Get() };
		get_native()->ExecuteCommandLists(_countof(s), s);
		//	signal_and_wait_internal();
		return signal_internal();
	}
	FenceWaiter Queue::execute_internal(CommandList* list)
	{
		PROFILE(L"execute_internal");

		if (stop || !Device::get().alive)
			return { &commandListCounter,m_fenceValue };

		std::unique_lock<std::mutex> lock(queue_mutex);


		auto cl = list->get_ptr();

		{
			PROFILE(L"update_tile_mappings");

			bool has_updates = false;

			auto& updates = list->tile_updates;
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
						auto& updates = list->tile_updates;
						for (auto& u : updates)
							u.resource->tracked_info->on_tile_update(u);
						updates.clear();
					});
			}
		}


		auto transition_list = list->fix_pretransitions();

		if (transition_list)
		{
			if (transition_list->get_type() == list->get_type())
			{
				ID3D12CommandList* s[] = { transition_list->get_native().Get(), list->compiled.m_commandList.Get() };
				native->ExecuteCommandLists(_countof(s), s);
			}
			else
			{

				// Need to request other queue to make a proper transition.
				// It's OK, but better to avoid this
				auto queue = Device::get().get_queue(transition_list->get_type());
				auto waiter = queue->run_transition_list(last_executed_fence, transition_list);

				gpu_wait_internal(waiter);

				{
					ID3D12CommandList* s[] = { list->compiled.m_commandList.Get() };
					native->ExecuteCommandLists(_countof(s), s);
				}
			}

		}
		else
		{
			ID3D12CommandList* s[] = { list->compiled.m_commandList.Get() };
			native->ExecuteCommandLists(_countof(s), s);
		}
		//	signal_and_wait_internal();
		const FenceWaiter execution_fence = signal_internal();

		{
			PROFILE(L"on_send");
			list->on_send(execution_fence);
			list->free_resources();
		}


		executor.enqueue([cl, this]()
			{
				if (!Device::get().alive) return;

				auto list = cl;
				list->get_execution_fence().get().wait();
				PROFILE(L"on_execute");
				list->on_execute();
			});

		last_executed_fence = execution_fence;
		return execution_fence;
	}


	FenceWaiter Queue::execute(std::shared_ptr<CommandList> list)
	{
		assert(this->type == list->type);
		return execute_internal(list.get());
	}


	FenceWaiter Queue::execute(CommandList* list)
	{
		return execute(list->get_ptr());
	}


}
