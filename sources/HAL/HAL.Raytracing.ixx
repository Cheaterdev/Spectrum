export module HAL:Raytracing;

import :VirtualBuffer;
import :ResourceViews;

// MaterialAsset belongs to RenderSystem; a second declaration here would be attached to HAL and clash with it (Clang).

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

			// A BLAS built from a BlasStorage lives in a range of the storage's shared buffers instead of buffers of its
			// own; the structure keeps them alive. cur_buffer is null for it and blas_address names the range.
			std::shared_ptr<Buffer> blas_result;
			std::shared_ptr<Buffer> blas_scratch;
			ResourceAddress blas_address;
		public:
			StructuredBufferView<std::byte> *cur_buffer = nullptr;
			using ptr = std::shared_ptr<RaytracingAccelerationStructure>;

			void* material;   // opaque RenderSystem MaterialAsset*

			// Result and scratch memory for many BLASes built together (one MeshAsset::init_gpu): a few buffers, growing
			// in size, cut into 256-byte aligned ranges, instead of two resources per structure. Scratch ranges are never
			// reused, so builds in one command list need no barrier between them. A backend that cannot place a
			// structure inside a larger buffer (Device::shares_blas_storage() false) ignores it and creates buffers per
			// structure.
			class BlasStorage
			{
			public:
				struct Range
				{
					std::shared_ptr<Buffer> buffer;
					uint64 offset = 0;
				};

			private:
				struct Chunk
				{
					std::shared_ptr<Buffer> buffer;
					uint64 used = 0;
					uint64 capacity = 0;
					uint64 next_size = 0;
				};

				Device& device;
				Chunk result;
				Chunk scratch;

				Range allocate(Chunk& chunk, uint64 size, uint64 chunk_size, const char* name);

			public:
				explicit BlasStorage(Device& device) : device(device) {}

				Range allocate_result(uint64 size);
				Range allocate_scratch(uint64 size);
			};

			RaytracingAccelerationStructure(Device& device, std::vector<HAL::GeometryDesc>  desc, CommandList::ptr list, BlasStorage* storage = nullptr);


			RaytracingAccelerationStructure(Device& device, std::vector<HAL::InstanceDesc> instances);


			void new_frame();
			void update(CommandList::ptr list, UINT size, HAL::ResourceAddress address, bool need_rebuild);

			HAL::ResourceAddress get_gpu_address() const;
			HLSL::RaytracingAccelerationStructure get_handle() const;
		};


	}
}