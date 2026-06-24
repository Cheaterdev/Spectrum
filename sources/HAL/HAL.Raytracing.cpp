module HAL:Raytracing;
import :Queue;
import :Device;

using namespace HAL;


HAL::RaytracingAccelerationStructure::RaytracingAccelerationStructure(Device& device, std::vector<GeometryDesc> desc,
	CommandList::ptr list)
{
	HAL::RaytracingBuildDescBottomInputs inputs;
	inputs.Flags = RaytracingBuildFlags::PREFER_FAST_TRACE;
	inputs.geometry = desc;

	auto bottomLevelPrebuildInfo = device.calculateBuffers(inputs);

	scratch_buffer = StructuredBufferView<std::byte>(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, counterType::NONE,  HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource, HeapType::DEFAULT);
	prev_buffer = StructuredBufferView<std::byte>(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, counterType::NONE,  HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource, HeapType::DEFAULT);
	cur_buffer = &prev_buffer;
	HAL::RaytracingBuildDescStructure bottomLevelBuildDesc;
	{
		bottomLevelBuildDesc.DestAccelerationStructureData = prev_buffer.get_resource_address();
		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch_buffer.get_resource_address();
	}

	list->get_compute().build_ras(bottomLevelBuildDesc, inputs);
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


	currentResource = std::make_shared<virtual_gpu_buffer<std::byte>>(device, 1024 * 1024 * 256, counterType::NONE, HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource);
	prevResource = std::make_shared<virtual_gpu_buffer<std::byte>>(device, 1024 * 1024 * 256, counterType::NONE, HAL::ResFlags::Raytracing | HAL::ResFlags::ShaderResource);


	scratchInfo = std::make_shared<virtual_gpu_buffer<std::byte>>(device, 1024 * 1024 * 16, counterType::NONE, HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource);
	scratchInfo->reserve(*list, topLevelPrebuildInfo.ScratchDataSizeInBytes);
	currentResource->reserve(*list, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);


	RaytracingBuildDescStructure topLevelBuildDesc;
	{
		topLevelBuildDesc.DestAccelerationStructureData = currentResource->buffer.get_resource_address();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer.get_resource_address();
	}

	list->get_compute().build_ras(topLevelBuildDesc, inputs);

	list->execute_and_wait();

	handle_table = device.get_static_gpu_data().alloc_descriptor(2, DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });

	raytracing_handles[0] = HLSL::RaytracingAccelerationStructure(handle_table[0]);
	raytracing_handles[0].create(currentResource->buffer.resource);
	raytracing_handles[1] = HLSL::RaytracingAccelerationStructure(handle_table[1]);
	raytracing_handles[1].create(prevResource->buffer.resource);
	current = 0;


	scratch_buffer = scratchInfo->buffer;
	cur_buffer = &currentResource->buffer;
	prev_buffer = prevResource->buffer;

}

void HAL::RaytracingAccelerationStructure::update(CommandList::ptr list, UINT size, ResourceAddress address,
	bool need_rebuild)
{
	//	return;
	//	if (resource) return;

	RaytracingBuildFlags buildFlags = RaytracingBuildFlags::ALLOW_UPDATE;
	need_rebuild = true;
	if (!need_rebuild)
		buildFlags |= RaytracingBuildFlags::PERFORM_UPDATE;


	std::swap(prevResource, currentResource);

	HAL::RaytracingBuildDescTopInputs inputs;
	inputs.Flags = RaytracingBuildFlags::ALLOW_UPDATE;
	inputs.NumDescs = size;
	inputs.instances = address;

	auto topLevelPrebuildInfo = currentResource->buffer.resource->get_device().calculateBuffers(inputs);

	UINT64 max = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	if (!need_rebuild) max = topLevelPrebuildInfo.UpdateScratchDataSizeInBytes;



	scratchInfo->reserve(*list, max);


	currentResource->reserve(*list, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	//auto instanceDescs = list->place_raw(instances);

	HAL::RaytracingBuildDescStructure topLevelBuildDesc;
	{
		topLevelBuildDesc.DestAccelerationStructureData = currentResource->buffer.get_resource_address();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer.get_resource_address();

		if (!need_rebuild)
			topLevelBuildDesc.SourceAccelerationStructureData = prevResource->buffer.get_resource_address();
		else
			topLevelBuildDesc.SourceAccelerationStructureData = ResourceAddress();

	}
	list->get_compute().build_ras(topLevelBuildDesc, inputs);
}

HAL::ResourceAddress HAL::RaytracingAccelerationStructure::get_gpu_address() const
{
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