module HAL:API.Queue;


import d3d12;
import Core;
import HAL;
#undef THIS

using namespace HAL;
namespace HAL
{

	Queue::Queue(CommandListType type, Device* device) : commandListCounter(*device), type(type)
	{
		API::Queue::construct(type, device);
		m_fenceValue = 0;
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
				(infos.resource)->get_dx(),
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

			auto dx_target_resource = (infos.resource)->get_dx();
			auto dx_source_resource = (infos.source)->get_dx();
			native->CopyTileMappings(dx_target_resource, &target, dx_source_resource, &source, &TRS, D3D12_TILE_MAPPING_FLAG_NONE);
		}
	
	}

	//	signal_and_wait_internal();
	ClockCalibrationInfo Queue::get_clock_time() const
	{
		UINT64 cpu_start;
		UINT64 gpu_start;
		native->GetClockCalibration(&gpu_start, &cpu_start);
		return { cpu_start,gpu_start, frequency };
	}

	namespace API
	{
		void Queue::construct(HAL::CommandListType type, Device* device)
		{
			auto THIS = static_cast<HAL::Queue*>(this);

			auto t = CounterManager::get().start_count<Queue>();
		
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
			queueDesc.Type = ::to_native(type);
			queueDesc.NodeMask = 1;
			device->native_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&native));

			auto name = std::string("DXQueue: ") + std::string(magic_enum::enum_name(type));

			native->SetName(convert(name).c_str());

			native->GetTimestampFrequency(&THIS->frequency);
		}

		void Queue::execute(CommandList* list)
		{
			ID3D12CommandList* s[] = { (list)->compiled.m_commandList.Get() };
			native->ExecuteCommandLists(_countof(s), s);
		}

		void Queue::execute(TransitionCommandList* list)
		{
			ID3D12CommandList* s[] = { (list)->get_native().Get() };
			native->ExecuteCommandLists(_countof(s), s);
		}

		D3D::CommandQueue Queue::get_native()
		{
			return native;
		}

		void  Queue::gpu_wait(FenceWaiter waiter)
		{
			if (!waiter) return;
			native->Wait(waiter.fence->m_fence.Get(), waiter.value);
		}

		void Queue::signal(Fence& fence, Fence::CounterType value)
		{
			native->Signal(fence.m_fence.Get(), value);
		}
	}
}
