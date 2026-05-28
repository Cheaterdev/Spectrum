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
					Device& device = *cereal::get_user_data<UniversalContext>(ar).get_context<Device*>();
					_init(device, desc, HeapType::DEFAULT, TextureLayout::UNDEFINED);
					GPUBinaryData<true> binary;
					ar& NVP(binary);
					Resource::write(binary);
					init();
				}
				else
				{
					auto data = read();
					GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ 0,desc.as_buffer().SizeInBytes }, data, get_device());
					ar& NVP(binary);
				}
			}
		};


	}


}
