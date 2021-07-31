#pragma once

class MaterialAsset;

namespace DX12
{


	class RaytracingAccelerationStructure
	{


		GPUBuffer::ptr scratchInfo;

		GPUBuffer::ptr resources[2];

		int current = 0;

		HandleTable handle_table;
	public:
		GPUBuffer::ptr resource;
		using ptr = std::shared_ptr<RaytracingAccelerationStructure>;
		HLSL::RaytracingAccelerationStructure raytracing_handle;

		MaterialAsset* material;
		RaytracingAccelerationStructure(std::vector<GeometryDesc>  desc, CommandList::ptr list)
		{
			RaytracingBuildDescBottomInputs inputs;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

			for (auto& e : desc)
				inputs.add_geometry(e);

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = Device::get().calculateBuffers(inputs);


			resource = std::make_shared<GPUBuffer>(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);
			scratchInfo = std::make_shared<GPUBuffer>(bottomLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS);

			RaytracingBuildDescStructure bottomLevelBuildDesc;
			{
				bottomLevelBuildDesc.DestAccelerationStructureData = resource->get_resource_address();
				bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_resource_address();
			}

			list->get_compute().build_ras(bottomLevelBuildDesc, inputs);
			resources[0] = resource;
		}


		RaytracingAccelerationStructure(std::vector<InstanceDesc> instances)
		{

			auto list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
			list->begin("RaytracingAccelerationStructure");

			
			RaytracingBuildDescTopInputs inputs;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
			inputs.NumDescs = instances.size();

			if (instances.size())
			{
				auto instanceDescs = list->place_raw(instances);
				inputs.instances = instanceDescs.get_resource_address();
			}
			
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = Device::get().calculateBuffers(inputs);
			

			resource = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);
			scratchInfo = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS);



			RaytracingBuildDescStructure topLevelBuildDesc;
			{
				topLevelBuildDesc.DestAccelerationStructureData = resource->get_resource_address();
				topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_resource_address();
			}
			
			list->get_compute().build_ras(topLevelBuildDesc, inputs);
			list->end();
			list->execute_and_wait();
			resources[0] = resource;

			handle_table = DescriptorHeapManager::get().get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV)->create_table(1);

			raytracing_handle = HLSL::RaytracingAccelerationStructure(handle_table[0]);
			raytracing_handle.create(resource.get());
		}


		void update(CommandList::ptr list, UINT size, ResourceAddress address, bool need_rebuild)
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
			need_rebuild = true;
			if (!need_rebuild)

				buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

			current = 1 - current;

			auto& prev = resources[current];
			auto& cur = resources[current];


			RaytracingBuildDescTopInputs inputs;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
			inputs.NumDescs = size;
			inputs.instances = address;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = Device::get().calculateBuffers(inputs);

			if (!cur || cur->get_count() < topLevelPrebuildInfo.ResultDataMaxSizeInBytes)
				cur = std::make_shared<GPUBuffer>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::RAYTRACING_STRUCTURE);


			UINT64 max = topLevelPrebuildInfo.ScratchDataSizeInBytes;

			if (!need_rebuild) max = topLevelPrebuildInfo.UpdateScratchDataSizeInBytes;


			if (!scratchInfo || (scratchInfo->get_count() < max))
				scratchInfo = std::make_shared<GPUBuffer>(max, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS);

			//auto instanceDescs = list->place_raw(instances);

			RaytracingBuildDescStructure topLevelBuildDesc;
			{
				topLevelBuildDesc.DestAccelerationStructureData = resource->get_resource_address();
				topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_resource_address();

				if (!need_rebuild)
					topLevelBuildDesc.SourceAccelerationStructureData =	prev ? prev->get_resource_address() : ResourceAddress();
				else
					topLevelBuildDesc.ScratchAccelerationStructureData = scratchInfo->get_resource_address();
				
			}
			list->get_compute().build_ras(topLevelBuildDesc, inputs);

			resource = resources[current];

			raytracing_handle.create(resource.get());
		}

		ResourceAddress get_gpu_address() const
		{
			return resource->get_resource_address();
		}



	};


}