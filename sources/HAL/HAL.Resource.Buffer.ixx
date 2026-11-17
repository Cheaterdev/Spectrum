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
			std::vector<std::byte> read(uint64 offset, uint64 size);
			void write(std::vector<std::byte>&);

			// Where the buffer's contents sit in a file, as stored: the chunks it was loaded from, or the ones its last
			// save wrote. While the buffer has not been written since (stored_version), saving it copies those chunks
			// across instead of reading the buffer back from the GPU and compressing it again.
			struct StoredChunk
			{
				std::filesystem::path file;
				uint64 file_offset;         // of the stored bytes
				uint64 stored_size;
				uint64 uncompressed_size;
				uint64 offset;              // of the chunk in the buffer
				uint64 size;
			};
			std::vector<StoredChunk> stored_chunks;
			uint64 stored_version = 0;
			bool stored_chunks_usable(uint64 total) const;
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
					std::vector<StoredChunk> loaded;
					do
					{
						GPUBinaryData<true> binary;
						ar& NVP(binary);
						Resource::write(binary, false);
						if (const auto* range = std::get_if<GPUBinaryData<true>::Buffer>(&binary.desc))
							loaded.push_back({ binary.operation.path, binary.operation.file_offset, binary.operation.size, binary.uncompressed_size, range->offset, range->size });
						covered += binary.uncompressed_size;
					} while (covered < total);
					Resource::finish_writes();
					init();

					stored_chunks = std::move(loaded);
					stored_version = contents_version.load();
				}
				else
				{
					// One blob per chunk: a single GDeflate request for a whole big buffer (a 2000-part mesh asset is hundreds of MB)
					// decodes to all-zero on DirectStorage with no error, see DS_STAGING_BUFFER_SIZE in HAL.D3D12.Queue.cpp.
					constexpr uint64 CHUNK = 16 * 1024 * 1024;
					const uint64 total = desc.as_buffer().SizeInBytes;

					// Taken before the bytes are: a write recorded while this runs leaves the chunks noted below stale.
					const uint64 version = contents_version.load();
					FileDataStorage* const storage = FileDataStorage::writing();
					std::vector<StoredChunk> written;

					auto note = [&](const GPUBinaryData<false>& binary, uint64 offset, uint64 size)
					{
						if (storage && binary.operation.written_to_storage)
							written.push_back({ storage->get_path(), binary.operation.written_at, binary.operation.size, binary.uncompressed_size, offset, size });
					};

					if (stored_chunks_usable(total))
					{
						for (const StoredChunk& chunk : stored_chunks)
						{
							GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ chunk.offset, chunk.size }, chunk.file, chunk.file_offset, chunk.stored_size, chunk.uncompressed_size);
							ar& NVP(binary);
							note(binary, chunk.offset, chunk.size);
						}
					}
					else
					{
						// A chunk at a time: read back, compressed (the codec uses every core for one chunk) and written
						// before the next is fetched, so the whole buffer is never in memory.
						uint64 offset = 0;
						do
						{
							const uint64 size = std::min(CHUNK, total - offset);
							auto data = read(offset, size);
							GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ offset, size }, std::span<std::byte>(data.data(), data.size()), get_device());
							ar& NVP(binary);
							note(binary, offset, size);
							offset += size;
						} while (offset < total);
					}

					// The file just written holds the buffer only once the save has put it in place.
					if (storage && !written.empty())
						storage->on_commit([weak = weak_from_this(), written = std::move(written), version]() mutable
						{
							if (auto self = std::dynamic_pointer_cast<Buffer>(weak.lock()))
							{
								self->stored_chunks = std::move(written);
								self->stored_version = version;
							}
						});
				}
			}
		};


	}


}
