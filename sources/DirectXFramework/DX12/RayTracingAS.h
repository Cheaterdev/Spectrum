#pragma once

class MaterialAsset;

namespace DX12
{

	class RaytracingAccelerationStructure
	{
		
	
		GPUBuffer::ptr scratchInfo;

		GPUBuffer::ptr resources[2];

		int current = 0;
	public:
		GPUBuffer::ptr resource;
		using ptr = std::shared_ptr<RaytracingAccelerationStructure>;
		HandleTable srvs;
		MaterialAsset* material;
		RaytracingAccelerationStructure(std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>  desc)
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;


		//	std::vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS> inputs;
		//	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;


			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {};
			bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			bottomLevelInputs.Flags = buildFlags;
			bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			bottomLevelInputs.pGeometryDescs = desc.data();
			bottomLevelInputs.NumDescs = desc.size();


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
			Device::get().get_native_device()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);


			resource = std::make_shared<GPUBuffer>(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);
			scratchInfo = std::make_shared<GPUBuffer>(bottomLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS);
		


			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
			{
				bottomLevelBuildDesc.Inputs = bottomLevelInputs;
				bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_gpu_address();
				bottomLevelBuildDesc.DestAccelerationStructureData = resource->get_gpu_address();
			}

			auto list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
			list->begin();
			list->get_native_list()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
			list->transition_uav(resource.get());
			list->end();
			list->execute_and_wait();

			resources[0] = resource;

		}


		RaytracingAccelerationStructure(std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances)
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;


			//	std::vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS> inputs;
			//	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;


			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
			topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			topLevelInputs.Flags = buildFlags;
			topLevelInputs.NumDescs = instances.size();
			topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
			Device::get().get_native_device()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);


			resource = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);
			scratchInfo = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS);
		
			
			auto list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
			list->begin();

			auto instanceDescs = list->place_raw(instances);

			// Top Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
			{
				topLevelInputs.InstanceDescs = instanceDescs.get_gpu_address();
				topLevelBuildDesc.Inputs = topLevelInputs;
				topLevelBuildDesc.DestAccelerationStructureData = resource->get_gpu_address();
				topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_gpu_address();
			}



			list->get_native_list()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
			list->end();
			list->execute_and_wait();
			resources[0] = resource;

		}


		void update(CommandList::ptr list, std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances)
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

			auto prev = resources[1-current];
			current = 1 - current;
			auto cur = resource;

			//	std::vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS> inputs;
			//	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;


			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
			topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			topLevelInputs.Flags = buildFlags;
			topLevelInputs.NumDescs = instances.size();
			topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
			Device::get().get_native_device()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

			if (!cur || cur->get_size() < topLevelPrebuildInfo.ResultDataMaxSizeInBytes);
			cur = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);

		//	scratchInfo = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS);


			auto instanceDescs = list->place_raw(instances);

			// Top Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
			{
				topLevelInputs.InstanceDescs = instanceDescs.get_gpu_address();
				topLevelBuildDesc.Inputs = topLevelInputs;
				topLevelBuildDesc.DestAccelerationStructureData = resource->get_gpu_address();
				topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_gpu_address();
				topLevelBuildDesc.SourceAccelerationStructureData =
					prev ? prev->get_gpu_address() : 0;
			}




			list->transition(cur.get(), ResourceState::UNORDERED_ACCESS);

			list->get_native_list()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);


			resource = resources[current];
		}

		D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address() const
		{
			return resource->get_gpu_address();
		}



	};


}