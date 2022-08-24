export module Graphics:RaytracingAS;

import :Buffer;
import :VirtualBuffer;
import :Queue;
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

			//GPUBuffer::ptr resource;
			using ptr = std::shared_ptr<RaytracingAccelerationStructure>;
			HLSL::RaytracingAccelerationStructure raytracing_handle;

			MaterialAsset* material;


			RaytracingAccelerationStructure(std::vector<GeometryDesc>  desc, CommandList::ptr list);


			RaytracingAccelerationStructure(std::vector<InstanceDesc> instances);


			void update(CommandList::ptr list, UINT size, ResourceAddress address, bool need_rebuild);

			ResourceAddress get_gpu_address() const;
		};


	}
}