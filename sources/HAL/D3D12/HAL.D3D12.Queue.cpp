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

	DirectStorageQueue::DirectStorageQueue(Device& device) :requestCounter(device)
	{
		    DSTORAGE_CONFIGURATION config{};
    config.DisableGpuDecompression = false;
   TEST(device, DStorageSetConfiguration(&config));

	
		TEST(device, DStorageGetFactory(IID_PPV_ARGS(&factory)));
		if constexpr (Debug::CheckErrors)    factory->SetDebugFlags(DSTORAGE_DEBUG_BREAK_ON_ERROR | DSTORAGE_DEBUG_SHOW_ERRORS);
    factory->SetStagingBufferSize(256 * 1024 * 1024);


		// Create a DirectStorage queue which will be used to load data into a
		// buffer on the GPU.
		DSTORAGE_QUEUE_DESC queueDesc{};
		queueDesc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY;
		queueDesc.Priority = DSTORAGE_PRIORITY_NORMAL;
		queueDesc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
		queueDesc.Device = device.get_native_device().Get();

		TEST(device, factory->CreateQueue(&queueDesc, IID_PPV_ARGS(&native)));


	}
	DirectStorageQueue::~DirectStorageQueue()
	{

	}

		void DirectStorageQueue::flush()
	{
			std::lock_guard<std::mutex> g(queue_mutex);
			
			 native->Submit();
	}
	void DirectStorageQueue::stop_all()
	{

	}
	HAL::FenceWaiter DirectStorageQueue::signal()
	{
		auto value = ++m_fenceValue;
		native->EnqueueSignal(requestCounter.m_fence.Get(), value);

		return FenceWaiter{ &requestCounter, value };
	}
		void DirectStorageQueue::signal_and_wait()
	{
		auto s = signal();
		flush();
		s.wait();
	}
	bool DirectStorageQueue::is_complete(UINT64 fence)
	{
		return requestCounter.get_completed_value() >= fence;
	}

			FenceWaiter DirectStorageQueue::get_waiter()
	{
			return FenceWaiter{ &requestCounter, 0 };
	}
	HAL::FenceWaiter DirectStorageQueue::execute(StorageRequest srequest)
	{

	 D3D::StorageFile file;
    
    HRESULT hr = factory->OpenFile(srequest.file.wstring().c_str(), IID_PPV_ARGS(&file));



		 DSTORAGE_REQUEST request = {};
    request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;

		if(srequest.compressed)
		{
			 request.Options.CompressionFormat = DSTORAGE_COMPRESSION_FORMAT_GDEFLATE;

		}
		
    request.Source.File.Source = file.Get();
    request.Source.File.Offset = srequest.file_offset;
    request.Source.File.Size = srequest.size;
    request.UncompressedSize = srequest.uncompressed_size;



	std::visit(overloaded{
				[&](const StorageRequest::Buffer& buffer) {
					request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;
		request.Destination.Buffer.Resource = srequest.resource->get_dx();
		request.Destination.Buffer.Offset = buffer.offset;
		request.Destination.Buffer.Size = request.UncompressedSize;
				},
				[&](const StorageRequest::Texture& texture) {
					request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_TEXTURE_REGION;
	//	request.Destination.MultipleSubresources.Resource = srequest.resource->get_dx();
	//request.Destination.MultipleSubresources.FirstSubresource = texture.subresource;


					 request.Destination.Texture.Resource =  srequest.resource->get_dx();
        request.Destination.Texture.SubresourceIndex = texture.subresource;

					auto size=srequest.resource->get_desc().as_texture().get_size(texture.subresource);
        D3D12_BOX destBox{};
        destBox.right = size.x;
        destBox.bottom = size.y;
        destBox.back =  size.z;

        request.Destination.Texture.Region = destBox;




				/*	auto l = HAL::Device::get().get_texture_layout(srequest.resource->get_desc(), texture.subresource);
					assert(l.size==srequest.size);*/
				},
				[&](auto other) {
					assert(false);
				}
		}, srequest.operation);

			std::lock_guard<std::mutex> g(queue_mutex);
			


	  native->EnqueueRequest(&request);

			auto waiter = signal();



		executor.enqueue([waiter, f=file]() {
				waiter.wait();
	f->Close();
				});

	//	 native->Submit();
	//		s.wait();

//		   DSTORAGE_ERROR_RECORD errorRecord{};
 //   native->RetrieveErrorRecord(&errorRecord);

		return waiter;
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
