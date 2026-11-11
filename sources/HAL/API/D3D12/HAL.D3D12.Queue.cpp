module HAL:API.Queue;

import d3d12;
import Core;
import HAL;
#undef THIS

namespace HAL
{
    // A GPU-decompressed DirectStorage request whose compressed size approaches
    // or exceeds the staging buffer silently decodes to all-zero (no error, no
    // validation warning) instead of failing loudly - this is what caused the
    // 8192^2-texture mip-0-black bug. Assert instead of letting that recur.
    static constexpr UINT64 DS_STAGING_BUFFER_SIZE = 128 * 1024 * 1024;

    // NOTE: the shared HAL::Queue(CommandListType, Device&) constructor is
    // defined in the backend-neutral sources/HAL/HAL.Queue.cpp (it just calls
    // API::Queue::construct).  Do not redefine it here — doing so produces an
    // LNK2005 duplicate-symbol error at executable link time.

    void Queue::update_tile_mappings(const update_tiling_info& infos)
    {
        PROFILE(L"update_tile_mappings");
        std::vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates;
        std::vector<D3D12_TILE_REGION_SIZE> regionSizes;
        std::vector<D3D12_TILE_RANGE_FLAGS> rangeFlags;
        std::vector<UINT> heapRangeStartOffsets;
        std::vector<UINT> rangeTileCounts;

        for (auto& [heap, tiles] : infos.tiles)
        {
            startCoordinates.clear();
            regionSizes.clear();
            rangeFlags.clear();
            heapRangeStartOffsets.clear();
            rangeTileCounts.clear();
            startCoordinates.reserve(tiles.size());
            regionSizes.reserve(tiles.size());
            rangeFlags.reserve(tiles.size());
            heapRangeStartOffsets.reserve(tiles.size());
            rangeTileCounts.reserve(tiles.size());

            const ResourceTile* previous = nullptr;   // the tile the last region ends with, while that region can still grow

            for (const ResourceTile& tile : tiles)
            {
                // A tile that follows the previous one in the same row of the same subresource, in the next tile of the
                // heap, extends its region: the region is NumTiles tiles in the subresource's x-then-y order, so
                // a neighbour in x is the next one. Only single tiles are merged (a longer one may wrap onto the next
                // row), and never unmaps: they have no heap tile to be consecutive in.
                if (previous && heap && previous->heap_position.count == 1 && tile.heap_position.count == 1
                    && tile.subresource == previous->subresource && tile.pos.y == previous->pos.y && tile.pos.z == previous->pos.z
                    && tile.pos.x == previous->pos.x + int(regionSizes.back().NumTiles)
                    && tile.heap_position.offset == previous->heap_position.offset + regionSizes.back().NumTiles)
                {
                    regionSizes.back().NumTiles++;
                    rangeTileCounts.back()++;
                    continue;
                }

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
                previous = &tile;
            }

            native->UpdateTileMappings(
                (infos.resource)->get_dx(),
                UINT(startCoordinates.size()),
                startCoordinates.data(),
                regionSizes.data(),
                heap ? heap->native_heap.Get() : nullptr,
                UINT(rangeFlags.size()),
                rangeFlags.data(),
                heapRangeStartOffsets.data(),
                rangeTileCounts.data(),
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

    ClockCalibrationInfo Queue::get_clock_time() const
    {
        UINT64 cpu_start;
        UINT64 gpu_start;
        native->GetClockCalibration(&gpu_start, &cpu_start);
        return { cpu_start, gpu_start, frequency };
    }

    DirectStorageQueue::DirectStorageQueue(Device& device) : device(device), requestCounter(device)
    {
        TEST(device, DStorageGetFactory(IID_PPV_ARGS(&factory)));
        if constexpr (Debug::CheckErrors)
            factory->SetDebugFlags(DSTORAGE_DEBUG_BREAK_ON_ERROR | DSTORAGE_DEBUG_SHOW_ERRORS);
        // 32MB was silently dropping GPU-decompressed subresources whose request
        // size approached/exceeded it (observed: an 8192^2 mip, ~21MB compressed /
        // 64MB uncompressed, decoded to all-zero while every smaller subresource
        // of the same texture was fine). 128MB gives headroom for large top mips.
        factory->SetStagingBufferSize(static_cast<uint32_t>(DS_STAGING_BUFFER_SIZE));

        // Create a DirectStorage queue which will be used to load data into a
        // buffer on the GPU.
        DSTORAGE_QUEUE_DESC queueDesc{};
        queueDesc.Capacity = DStorage::MAX_QUEUE_CAPACITY;
        queueDesc.Priority = DSTORAGE_PRIORITY_NORMAL;
        queueDesc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        queueDesc.Device = device.get_native_device().Get();

        TEST(device, factory->CreateQueue(&queueDesc, IID_PPV_ARGS(&native)));
        auto res = native->GetCompressionSupport(DSTORAGE_COMPRESSION_FORMAT_GDEFLATE);
        Log::get() << res << Log::endl;
    }

    DirectStorageQueue::~DirectStorageQueue()
    {
        executor.stop_and_wait();
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

    // One spelling per file, so close_file() finds what a request opened.
    static std::wstring storage_file_key(const std::filesystem::path& path)
    {
        std::error_code ec;
        auto absolute = std::filesystem::absolute(path, ec);
        return (ec ? path : absolute).lexically_normal().wstring();
    }

    namespace API
    {
        D3D::StorageFile DirectStorageQueue::cached_file(const std::filesystem::path& path)
        {
            const std::wstring key = storage_file_key(path);
            if (auto it = files.find(key); it != files.end())
                return it->second;

            D3D::StorageFile file;
            HRESULT hr = factory->OpenFile(key.c_str(), IID_PPV_ARGS(&file));
            if (FAILED(hr))
            {
                Log::get() << "DirectStorage: cannot open " << path.string() << " (hr " << static_cast<uint32_t>(hr) << ")" << Log::endl;
                return {};
            }
            files.emplace(key, file);
            return file;
        }
    }

    // Builds and enqueues one request; queue_mutex held. Nothing is signalled or submitted.
    static void enqueue_storage_request(Device& device, IDStorageQueue2* queue, IDStorageFile* file, const StorageRequest& srequest)
    {
        DSTORAGE_REQUEST request = {};
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;

        if (srequest.compressed)
        {
            request.Options.CompressionFormat = DSTORAGE_COMPRESSION_FORMAT_GDEFLATE;
        }

        request.Source.File.Source = file;
        request.Source.File.Offset = srequest.file_offset;
        request.Source.File.Size = static_cast<uint>(srequest.size);
        request.UncompressedSize = static_cast<uint>(srequest.uncompressed_size);

        ASSERT(request.Source.File.Size == srequest.size);
        ASSERT(request.UncompressedSize == srequest.uncompressed_size);

        // A GPU-decompressed (compressed==true) request whose compressed size is
        // at/above the staging buffer decodes to all-zero with no HRESULT error
        // and no debug-layer warning - see DS_STAGING_BUFFER_SIZE above. Assert
        // on the compressed size, since that's what's DMA'd into the staging
        // buffer; if this trips for a request whose compressed size looks safely
        // under the limit, the real constraint may be the uncompressed size or a
        // multi-request-in-flight sum instead - check that before just raising
        // the limit again.
        if (srequest.compressed)
            ASSERT(srequest.size < DS_STAGING_BUFFER_SIZE);

        std::visit(overloaded{
            [&](const StorageRequest::Buffer& buffer)
            {
                request.Options.DestinationType        = DSTORAGE_REQUEST_DESTINATION_BUFFER;
                request.Destination.Buffer.Resource    = srequest.resource->get_dx();
                request.Destination.Buffer.Offset      = buffer.offset;
                request.Destination.Buffer.Size        = request.UncompressedSize;
            },
            [&](const StorageRequest::Texture& texture)
            {
                // Serialized texture payloads are complete copyable footprints,
                // never partial regions.  The full-subresource path is also
                // required for BC tail mips: region copies expand a 2x2 mip to
                // its 4x4 compression block, which exceeds that mip's bounds.
                request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_MULTIPLE_SUBRESOURCES_RANGE;
                request.Destination.MultipleSubresourcesRange.Resource = srequest.resource->get_dx();
                request.Destination.MultipleSubresourcesRange.FirstSubresource = texture.subresource;
                request.Destination.MultipleSubresourcesRange.NumSubresources = texture.count;

                auto l = device.get_texture_range_layout(srequest.resource->get_desc(), texture.subresource, texture.count);
                ASSERT(l.total_size == srequest.uncompressed_size);
            },
            [&](const StorageRequest::Tiles& tiles)
            {
                request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_TILES;
                request.Destination.Tiles.Resource = srequest.resource->get_dx();
                request.Destination.Tiles.TiledRegionStartCoordinate = {};
                request.Destination.Tiles.TiledRegionStartCoordinate.X = tiles.first_tile.x;
                request.Destination.Tiles.TiledRegionStartCoordinate.Y = tiles.first_tile.y;
                request.Destination.Tiles.TiledRegionStartCoordinate.Z = tiles.first_tile.z;
                request.Destination.Tiles.TiledRegionStartCoordinate.Subresource = tiles.subresource;
                request.Destination.Tiles.TileRegionSize = {};
                request.Destination.Tiles.TileRegionSize.NumTiles = tiles.tile_count.x * tiles.tile_count.y * tiles.tile_count.z;
                request.Destination.Tiles.TileRegionSize.UseBox = TRUE;
                request.Destination.Tiles.TileRegionSize.Width = tiles.tile_count.x;
                request.Destination.Tiles.TileRegionSize.Height = static_cast<UINT16>(tiles.tile_count.y);
                request.Destination.Tiles.TileRegionSize.Depth = static_cast<UINT16>(tiles.tile_count.z);
            },
            [&](auto other)
            {
                ASSERT(false);
            }
        }, srequest.operation);

        queue->EnqueueRequest(&request);
    }

    HAL::FenceWaiter DirectStorageQueue::execute(StorageRequest srequest)
    {
        PROFILE(L"DirectStorageQueue::execute");

        // Not through the file cache: this path loads whole resources from the texture and shader cache files, which are
        // read once and may be rewritten at any time (hot reload). Only the batched path, which reads the same asset
        // file page after page, keeps its files open.
        D3D::StorageFile file;
        HRESULT hr = factory->OpenFile(srequest.file.wstring().c_str(), IID_PPV_ARGS(&file));
        if (FAILED(hr))
            Log::get() << "DirectStorage: cannot open " << srequest.file.string() << " (hr " << static_cast<uint32_t>(hr) << ")" << Log::endl;

        std::lock_guard<std::mutex> g(queue_mutex);
        enqueue_storage_request(device, native.Get(), file.Get(), srequest);
        auto waiter = signal();

        executor.enqueue([waiter, f = file]() {
            waiter.wait();
            if (f)
                f->Close();
        });

        return waiter;
    }

    HAL::FenceWaiter DirectStorageQueue::execute(std::span<const StorageRequest> requests)
    {
        PROFILE(L"DirectStorageQueue::execute_batch");

        std::lock_guard<std::mutex> g(queue_mutex);
        for (const StorageRequest& srequest : requests)
            enqueue_storage_request(device, native.Get(), cached_file(srequest.file).Get(), srequest);

        auto waiter = signal();
        native->Submit();
        return waiter;
    }

    void DirectStorageQueue::close_file(const std::filesystem::path& path)
    {
        D3D::StorageFile file;
        FenceWaiter waiter;
        {
            std::lock_guard<std::mutex> g(queue_mutex);
            auto it = files.find(storage_file_key(path));
            if (it == files.end())
                return;

            file = std::move(it->second);
            files.erase(it);

            // Requests made of it may be queued or in flight still: it closes once they are done.
            waiter = signal();
            native->Submit();
        }
        waiter.wait();
        file->Close();
    }

    namespace API
    {
        void Queue::construct(HAL::CommandListType type, Device* device)
        {
            auto THIS = static_cast<HAL::Queue*>(this);

            auto t = CounterManager::get().start_count<Queue>();

            // Describe and create the command queue.
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};

      /*      if (type == CommandListType::DIRECT)
            {
                queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
            }
            else
            {
                queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            }*/
                                  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
            queueDesc.Type = ::to_native(type);
            queueDesc.NodeMask = 1;
            device->native_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&native));

            auto name = std::string("DXQueue: ") + std::string(magic_enum::enum_name(type));

            native->SetName(convert(name).c_str());

            native->GetTimestampFrequency(&THIS->frequency);
        }

        void Queue::execute(const API::CommandList* list)
        {
            PROFILE(L"Queue::execute");

            // Accumulate lists into one ExecuteCommandLists — no per-list flush.
            // Legal because CommandListGroup::compile_transitions chains resource
            // state across list boundaries within a group: D3D12 forbids, within one
            // scope, accessing a resource after a SyncAfter == SYNC_NONE barrier or
            // emitting a SyncBefore == SYNC_NONE barrier once it has been accessed
            // (#1417). gpu_wait/signal still flush at real sync points.
            queued.emplace_back(list->get_native().Get());
        }

        void Queue::flush()
        {
            PROFILE(L"Queue::flush");
            if (!queued.empty())
            {
                native->ExecuteCommandLists(uint(queued.size()), queued.data());

                PROFILE(L"Queue::clear");
                queued.clear();
            }
        }

        D3D::CommandQueue Queue::get_native()
        {
            return native;
        }

        void Queue::gpu_wait(FenceWaiter waiter)
        {
            if (!waiter) return;
            PROFILE(L"Queue::gpu_wait");
            flush();
            native->Wait(waiter.fence->m_fence.Get(), waiter.value);
        }

        void Queue::signal(Fence& fence, Fence::CounterType value)
        {
            PROFILE(L"Queue::signal");
            flush();
            native->Signal(fence.m_fence.Get(), value);
        }
    }
}
