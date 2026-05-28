export module HAL:Resource.Buffer;
import <HAL.h>;
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

			Buffer(const ResourceDesc& desc, HeapType heap_type);
			Buffer(const ResourceDesc& desc, PlacementAddress handle);
			Buffer(const ResourceDesc& desc, ResourceHandle handle, bool own = false);
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
					_init(desc, HeapType::DEFAULT, TextureLayout::UNDEFINED);
					GPUBinaryData<true> binary;
					ar& NVP(binary);
					Resource::write(binary);
					init();
				}
				else
				{
					auto data = read();
					GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ 0,desc.as_buffer().SizeInBytes }, data);
					ar& NVP(binary);
				}
			}
		};


	}


}
