export module Graphics:RaytracingAS;

import :Buffer;
import :VirtualBuffer;

class MaterialAsset;

export
{


	namespace Graphics
	{


		class RaytracingAccelerationStructure
		{


			virtual_gpu_buffer<std::byte>::ptr scratchInfo;


			virtual_gpu_buffer<std::byte>::ptr prevResource;

			int current = 0;

			HandleTable handle_table;
		public:
			virtual_gpu_buffer<std::byte>::ptr currentResource;

			using ptr = std::shared_ptr<RaytracingAccelerationStructure>;
			HLSL::RaytracingAccelerationStructure raytracing_handle;

			MaterialAsset* material;


			RaytracingAccelerationStructure(std::vector<HAL::GeometryDesc>  desc, CommandList::ptr list);


			RaytracingAccelerationStructure(std::vector<HAL::InstanceDesc> instances);


			void update(CommandList::ptr list, UINT size, HAL::ResourceAddress address, bool need_rebuild);

			HAL::ResourceAddress get_gpu_address() const;
		};


	}
}