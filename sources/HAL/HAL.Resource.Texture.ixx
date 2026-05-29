export module HAL:Resource.Texture;

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


		class TextureResource: public Resource
		{
			void init();
		//	std::vector<std::byte> read();
		std::vector<std::byte> read(uint i);
		public:
			using ptr = std::shared_ptr<TextureResource>;
			TextureResource() = default; // NULL texture
			TextureResource(Device& device, const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));
			TextureResource(Device& device, const ResourceDesc& desc, PlacementAddress handle);
			TextureResource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			TextureResource(Device& device, const D3D::Resource& resouce, TextureLayout initialLayout);
			virtual ~TextureResource() {}
		private:

			std::vector<GPUBinaryData<true>> load_subresources; // TODO: make shared temp
			SERIALIZE()
			{
				serialize_from_derived = true;
				SAVE_PARENT(Resource);
		serialize_from_derived = false;
		

			
				if constexpr (Archive::is_loading::value)
				{
					Device* _dev = nullptr;
					if constexpr (can_get_context<Archive>)
						_dev = cereal::get_user_data<UniversalContext>(ar).template get_context<Device*>();
		
					Device& device = *_dev;
					_init(device, desc, HeapType::DEFAULT, TextureLayout::COPY_QUEUE);
					set_name("TextureResource::deserializing ");

						load_subresources.resize(desc.as_texture().Subresources());
						for (uint i = 0; i < desc.as_texture().Subresources(); i++)
						{
							//GPUBinaryData<true> binary;

							ar& NVP(load_subresources[i]);
							if (!desc.is_virtual())
								Resource::write(load_subresources[i]);

						}
					


				}
				else
				{
					
						for (uint i = 0; i < desc.as_texture().Subresources(); i++)
						{
							auto data = read(i);

							GPUBinaryData<false> binary(GPUBinaryData<false>::Texture{ i,1 }, data, get_device());

							ar& NVP(binary);
						}

					


				}
			}
		};

	}


}

