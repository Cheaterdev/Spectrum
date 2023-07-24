export module HAL:Raytracing;

import :Buffer;
import :VirtualBuffer;
import :ResourceViews;

class MaterialAsset;

export
{


	namespace HAL
	{


		class RaytracingAccelerationStructure
		{

			StructuredBufferView<std::byte> scratch_buffer;
			StructuredBufferView<std::byte> prev_buffer;
			
			virtual_gpu_buffer<std::byte>::ptr scratchInfo;
			virtual_gpu_buffer<std::byte>::ptr currentResource;


			virtual_gpu_buffer<std::byte>::ptr prevResource;

			int current = 0;

			Handle handle_table;
		public:
			StructuredBufferView<std::byte> *cur_buffer;
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