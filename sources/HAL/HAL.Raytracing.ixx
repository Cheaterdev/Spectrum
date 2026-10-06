export module HAL:Raytracing;

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
			StructuredBufferView<std::byte> prev_buffer;   // BLAS storage

			virtual_gpu_buffer<std::byte>::ptr scratchInfo;

			// TLAS storage, alternating per build so one frame's build never
			// overwrites a TLAS an earlier frame may still trace. Ordinary
			// buffers that grow, not reserved (sparse) ones: Vulkan can't create
			// an acceleration structure on a sparse-residency buffer.
			StructuredBufferView<std::byte> tlas_storage[2];
			void ensure_tlas_storage(Device& device, int index, uint64 size);

			// The storage this frame builds and reads. Chosen in new_frame() (pre-setup),
			// not in update(): update() runs in a pass's render, which records in parallel
			// with the passes that bind get_handle(), so flipping there races them.
			int current = 0;

			Handle handle_table;
			HLSL::RaytracingAccelerationStructure raytracing_handles[2];
		public:
			StructuredBufferView<std::byte> *cur_buffer;
			using ptr = std::shared_ptr<RaytracingAccelerationStructure>;

			MaterialAsset* material;


			RaytracingAccelerationStructure(Device& device, std::vector<HAL::GeometryDesc>  desc, CommandList::ptr list);


			RaytracingAccelerationStructure(Device& device, std::vector<HAL::InstanceDesc> instances);


			void new_frame();
			void update(CommandList::ptr list, UINT size, HAL::ResourceAddress address, bool need_rebuild);

			HAL::ResourceAddress get_gpu_address() const;
			HLSL::RaytracingAccelerationStructure get_handle() const;
		};


	}
}