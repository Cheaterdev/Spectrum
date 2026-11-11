module HAL:Raytracing;
import :Queue;
import :Device;

using namespace HAL;


HAL::RaytracingAccelerationStructure::RaytracingAccelerationStructure(Device& device, std::vector<GeometryDesc> desc,
	CommandList::ptr list, BlasStorage* storage)
{
	HAL::RaytracingBuildDescBottomInputs inputs;
	inputs.Flags = RaytracingBuildFlags::PREFER_FAST_TRACE;
	inputs.geometry = desc;

	auto bottomLevelPrebuildInfo = [&]
	{
		PROFILE(L"blas_prebuild_info");
		return device.calculateBuffers(inputs);
	}();

	HAL::RaytracingBuildDescStructure bottomLevelBuildDesc;
	{
		PROFILE(L"blas_create_buffers");
		if (storage && device.shares_blas_storage())
		{
			auto result = storage->allocate_result(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes);
			auto scratch = storage->allocate_scratch(bottomLevelPrebuildInfo.ScratchDataSizeInBytes);
			blas_result = result.buffer;
			blas_scratch = scratch.buffer;
			blas_address = blas_result->get_resource_address().offset(result.offset);
			bottomLevelBuildDesc.DestAccelerationStructureData = blas_address;
			bottomLevelBuildDesc.ScratchAccelerationStructureData = blas_scratch->get_resource_address().offset(scratch.offset);
		}
		else
		{
			scratch_buffer = StructuredBufferView<std::byte>(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, counterType::NONE,  HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource, HeapType::DEFAULT);
			prev_buffer = StructuredBufferView<std::byte>(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, counterType::NONE,  HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource, HeapType::DEFAULT);
			cur_buffer = &prev_buffer;
			bottomLevelBuildDesc.DestAccelerationStructureData = prev_buffer.get_resource_address();
			bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch_buffer.get_resource_address();
		}
	}

	{
		PROFILE(L"blas_build_record");
		list->get_compute().build_ras(bottomLevelBuildDesc, inputs);
	}
}

HAL::RaytracingAccelerationStructure::RaytracingAccelerationStructure(Device& device, std::vector<InstanceDesc> instances)
{



	auto list = (device.get_queue(CommandListType::DIRECT)->get_free_list());
	list->begin(L"RaytracingAccelerationStructure");


	HAL::RaytracingBuildDescTopInputs inputs;
	inputs.Flags = RaytracingBuildFlags::ALLOW_UPDATE;
	inputs.NumDescs = static_cast<UINT>(instances.size());

	if (instances.size())
	{
		inputs.instances = list->place_raw(instances);
	}

	auto topLevelPrebuildInfo = device.calculateBuffers(inputs);

	handle_table = device.get_static_gpu_data().alloc_descriptor(2, DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
	raytracing_handles[0] = HLSL::RaytracingAccelerationStructure(handle_table[0]);
	raytracing_handles[1] = HLSL::RaytracingAccelerationStructure(handle_table[1]);
	ensure_tlas_storage(device, 0, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);
	ensure_tlas_storage(device, 1, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	scratchInfo = std::make_shared<virtual_gpu_buffer<std::byte>>(device, 1024 * 1024 * 16, counterType::NONE, HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource);
	scratchInfo->reserve(*list, topLevelPrebuildInfo.ScratchDataSizeInBytes);

	RaytracingBuildDescStructure topLevelBuildDesc;
	{
		topLevelBuildDesc.DestAccelerationStructureData = tlas_storage[0].get_resource_address();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer.get_resource_address();
	}

	list->get_compute().build_ras(topLevelBuildDesc, inputs);

	list->execute_and_wait();

	current = 0;

	scratch_buffer = scratchInfo->buffer;
	cur_buffer = &tlas_storage[current];
}

void HAL::RaytracingAccelerationStructure::ensure_tlas_storage(Device& device, int index, uint64 size)
{
	auto& storage = tlas_storage[index];
	const uint64 capacity = storage.resource ? storage.get_desc().as_buffer().SizeInBytes : 0;
	if (capacity >= size && capacity) return;

	// Grown buffers double, so a growing scene reallocates only a few times.
	const uint64 new_size = std::max<uint64>({ size, capacity * 2, 64 * 1024 });
	storage = StructuredBufferView<std::byte>(device, new_size, counterType::NONE,
		HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource, HeapType::DEFAULT);
	storage.resource->set_name("TLAS");
	raytracing_handles[index].create(storage.resource);
}

void HAL::RaytracingAccelerationStructure::update(CommandList::ptr list, UINT size, ResourceAddress address,
	bool need_rebuild)
{
	//	return;
	//	if (resource) return;

	need_rebuild = true;


	const int prev = current ^ 1;

	HAL::RaytracingBuildDescTopInputs inputs;
	inputs.Flags = RaytracingBuildFlags::ALLOW_UPDATE;
	inputs.NumDescs = size;
	inputs.instances = address;

	auto& device = tlas_storage[prev].resource->get_device();
	auto topLevelPrebuildInfo = device.calculateBuffers(inputs);

	UINT64 max = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	if (!need_rebuild) max = topLevelPrebuildInfo.UpdateScratchDataSizeInBytes;

	scratchInfo->reserve(*list, max);
	ensure_tlas_storage(device, current, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);
	cur_buffer = &tlas_storage[current];

	HAL::RaytracingBuildDescStructure topLevelBuildDesc;
	{
		topLevelBuildDesc.DestAccelerationStructureData = tlas_storage[current].get_resource_address();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer.get_resource_address();

		if (!need_rebuild)
			topLevelBuildDesc.SourceAccelerationStructureData = tlas_storage[prev].get_resource_address();
		else
			topLevelBuildDesc.SourceAccelerationStructureData = ResourceAddress();

	}
	list->get_compute().build_ras(topLevelBuildDesc, inputs);
}

HAL::RaytracingAccelerationStructure::BlasStorage::Range HAL::RaytracingAccelerationStructure::BlasStorage::allocate(
	Chunk& chunk, uint64 size, uint64 chunk_size, const char* name)
{
	// The alignment D3D12 requires of an acceleration structure and of scratch memory.
	constexpr uint64 alignment = 256;
	size = std::max<uint64>((size + alignment - 1) / alignment * alignment, alignment);

	auto create = [&](uint64 bytes)
	{
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(bytes, HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource), HeapType::DEFAULT);
		buffer->set_name(name);
		return buffer;
	};

	// Bigger than a chunk: its own buffer, the chunk in use keeps its free tail.
	if (size > chunk_size)
		return { create(size), 0 };

	if (!chunk.buffer || chunk.used + size > chunk.capacity)
	{
		// Chunks start small and double up to chunk_size: most assets have a few small parts, and a full-size chunk
		// for each of them would hold tens of megabytes for nothing.
		constexpr uint64 first_chunk = 1024 * 1024;
		chunk.next_size = std::min(chunk_size, std::max(first_chunk, chunk.next_size * 2));
		chunk.capacity = std::max(chunk.next_size, size);
		chunk.buffer = create(chunk.capacity);
		chunk.used = 0;
	}

	Range range{ chunk.buffer, chunk.used };
	chunk.used += size;
	return range;
}

HAL::RaytracingAccelerationStructure::BlasStorage::Range HAL::RaytracingAccelerationStructure::BlasStorage::allocate_result(uint64 size)
{
	return allocate(result, size, 64ull * 1024 * 1024, "BLAS storage");
}

HAL::RaytracingAccelerationStructure::BlasStorage::Range HAL::RaytracingAccelerationStructure::BlasStorage::allocate_scratch(uint64 size)
{
	return allocate(scratch, size, 32ull * 1024 * 1024, "BLAS scratch");
}

HAL::ResourceAddress HAL::RaytracingAccelerationStructure::get_gpu_address() const
{
	if (!cur_buffer)
		return blas_address;

	return cur_buffer->get_resource_address();
}

void HAL::RaytracingAccelerationStructure::new_frame()
{
	current ^= 1;
}

HLSL::RaytracingAccelerationStructure HAL::RaytracingAccelerationStructure::get_handle() const
{
	return raytracing_handles[current];
}