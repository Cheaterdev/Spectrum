#include "pch_dx.h"
import RaytracingAS;
import Queue;
import HAL.Types;
import D3D12.Types;
using namespace HAL;


DX12::RaytracingAccelerationStructure::RaytracingAccelerationStructure(std::vector<GeometryDesc> desc,
	CommandList::ptr list)
{
	RaytracingBuildDescBottomInputs inputs;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	for (auto& e : desc)
		inputs.add_geometry(e);

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = Device::get().calculateBuffers(inputs);


	currentResource = std::make_shared<virtual_gpu_buffer<std::byte>>(1024 * 1024 * 256, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);
	//	prevResource = std::make_shared<virtual_gpu_buffer<std::byte>>(1024 * 1024 * 1024, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);

	scratchInfo = std::make_shared<virtual_gpu_buffer<std::byte>>(1024 * 1024 * 256, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			
	scratchInfo->reserve(*list, bottomLevelPrebuildInfo.ScratchDataSizeInBytes);
	currentResource->reserve(*list, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes);
	RaytracingBuildDescStructure bottomLevelBuildDesc;
	{
		bottomLevelBuildDesc.DestAccelerationStructureData = currentResource->buffer->get_resource_address();
		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer->get_resource_address();
	}

	list->get_compute().build_ras(bottomLevelBuildDesc, inputs);

}

DX12::RaytracingAccelerationStructure::RaytracingAccelerationStructure(std::vector<InstanceDesc> instances)
{

		

	auto list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
	list->begin("RaytracingAccelerationStructure");

			
	RaytracingBuildDescTopInputs inputs;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = static_cast<UINT>(instances.size());

	if (instances.size())
	{
		auto instanceDescs = list->place_raw(instances);
		inputs.instances = instanceDescs.get_resource_address();
	}
			
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = Device::get().calculateBuffers(inputs);
			

	currentResource = std::make_shared<virtual_gpu_buffer<std::byte>>(1024 * 1024 * 256, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);
	prevResource = std::make_shared<virtual_gpu_buffer<std::byte>>(1024 * 1024 * 256, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);

			
	scratchInfo = std::make_shared<virtual_gpu_buffer<std::byte>>(1024 * 1024*16 ,counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	scratchInfo->reserve(*list, topLevelPrebuildInfo.ScratchDataSizeInBytes);
	currentResource->reserve(*list, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);


	RaytracingBuildDescStructure topLevelBuildDesc;
	{
		topLevelBuildDesc.DestAccelerationStructureData = currentResource->buffer->get_resource_address();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer->get_resource_address();
	}
			
	list->get_compute().build_ras(topLevelBuildDesc, inputs);
	list->end();
	list->execute_and_wait();

	handle_table = StaticDescriptors::get().place(1);// DescriptorHeapManager::get().get_gpu_heap(DescriptorHeapType::CBV_SRV_UAV)->create_table(1);

	raytracing_handle = HLSL::RaytracingAccelerationStructure(handle_table[0]);
	raytracing_handle.create(currentResource->buffer.get());
}

void DX12::RaytracingAccelerationStructure::update(CommandList::ptr list, UINT size, ResourceAddress address,
	bool need_rebuild)
{
	//	return;
	//	if (resource) return;
			
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	need_rebuild = true;
	if (!need_rebuild)
		buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;


	std::swap(prevResource, currentResource);
			
	RaytracingBuildDescTopInputs inputs;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = size;
	inputs.instances = address;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = Device::get().calculateBuffers(inputs);
			
	UINT64 max = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	if (!need_rebuild) max = topLevelPrebuildInfo.UpdateScratchDataSizeInBytes;



	scratchInfo->reserve(*list, max);

			
	currentResource->reserve(*list, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	//auto instanceDescs = list->place_raw(instances);

	RaytracingBuildDescStructure topLevelBuildDesc;
	{
		topLevelBuildDesc.DestAccelerationStructureData = currentResource->buffer->get_resource_address();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->buffer->get_resource_address();

		if (!need_rebuild)
			topLevelBuildDesc.SourceAccelerationStructureData = prevResource->buffer->get_resource_address();
		else
			topLevelBuildDesc.SourceAccelerationStructureData = ResourceAddress();
				
	}
	list->get_compute().build_ras(topLevelBuildDesc, inputs);

		
	raytracing_handle.create(currentResource->buffer.get());
}

DX12::ResourceAddress DX12::RaytracingAccelerationStructure::get_gpu_address() const
{
	return currentResource->buffer->get_resource_address();
}
