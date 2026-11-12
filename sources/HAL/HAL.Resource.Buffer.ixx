export module HAL:Resource.Buffer;

import :HeapAllocators;
import :Device;
import :API.Resource;

import :TiledMemoryManager;
import :ResourceStates;
import :FrameManager;
import :Fence;
import :Resource;

import Core;


export{
	namespace HAL
	{

		class Buffer : public Resource
		{
			void init();
			std::vector<std::byte> read();
			void write(std::vector<std::byte>&);
		public:
			using ptr = std::shared_ptr<Buffer>;

			std::byte* buffer_data = nullptr;

			ResourceAddress gpu_address;
			Buffer() = default;

			Buffer(Device& device, const ResourceDesc& desc, HeapType heap_type);
			Buffer(Device& device, const ResourceDesc& desc, PlacementAddress handle);
			Buffer(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			// TODO:: works only for buffer now
			uint64 get_size() const;

			std::span<std::byte> cpu_data() const;

			ResourceAddress get_resource_address() const;

			virtual ~Buffer();
		private:
			SERIALIZE()
			{
		serialize_from_derived = true;
				SAVE_PARENT(Resource);
		serialize_from_derived = false;


				if constexpr (Archive::is_loading::value)
				{
					Device& device = *cereal::get_user_data<UniversalContext>(ar).template get_context<Device*>();
					_init(device, desc, HeapType::DEFAULT, TextureLayout::UNDEFINED);
					// Blobs follow until they cover the buffer; a file from before chunking holds one blob that already does.
					uint64 covered = 0;
					const uint64 total = desc.as_buffer().SizeInBytes;
					do
					{
						GPUBinaryData<true> binary;
						ar& NVP(binary);
						Resource::write(binary, false);
						covered += binary.uncompressed_size;
					} while (covered < total);
					Resource::finish_writes();
					init();
				}
				else
				{
					// One blob per chunk: a single GDeflate request for a whole big buffer (a 2000-part mesh asset is hundreds of MB)
					// decodes to all-zero on DirectStorage with no error, see DS_STAGING_BUFFER_SIZE in HAL.D3D12.Queue.cpp.
					constexpr uint64 CHUNK = 16 * 1024 * 1024;
					auto data = read();
					const uint64 total = desc.as_buffer().SizeInBytes;
					uint64 offset = 0;
					do
					{
						const uint64 size = std::min(CHUNK, total - offset);
						GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ offset, size }, std::span<std::byte>(data.data() + offset, size), get_device());
						ar& NVP(binary);
						offset += size;
					} while (offset < total);
				}
			}
		};


	}


}
