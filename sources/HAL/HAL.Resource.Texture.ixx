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
		// Reads [first_subresource, first_subresource+count) into one buffer laid
		// out per Device::get_texture_range_layout, so the whole range can be
		// compressed as a single blob (see SERIALIZE below).
		std::vector<std::byte> read_range(uint first_subresource, uint count);
		public:
			using ptr = std::shared_ptr<TextureResource>;
			TextureResource() = default; // NULL texture
			TextureResource(Device& device, const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));
			TextureResource(Device& device, const ResourceDesc& desc, PlacementAddress handle);
			TextureResource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			TextureResource(Device& device, const API::NativeImportHandle& handle, TextureLayout initialLayout);
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

						// One combined blob per array slice (its full mip chain), not one
						// per subresource: a lone single-subresource DirectStorage request
						// for a BC tail mip below the 4x4 compression block trips D3D12
						// error #858 (see DirectStorageQueue::execute's
						// MULTIPLE_SUBRESOURCES_RANGE comment). Batching the whole mip
						// chain keeps DirectStorage on the "whole range" placement path.
						const uint array_size = desc.as_texture().ArraySize;
						load_subresources.resize(array_size);
						for (uint a = 0; a < array_size; a++)
						{
							ar& NVP(load_subresources[a]);
							if (!desc.is_virtual())
								Resource::write(load_subresources[a]);

						}

						// Strict promote (deferred): register this freshly-uploaded
						// texture so the UploadManager transitions it out of COMMON
						// into its canonical read state at the next frame-begin.
						if (!desc.is_virtual())
							device.register_promote(get_ptr());
					


				}
				else
				{
					
						const uint mip_levels = desc.as_texture().MipLevels;
						const uint array_size = desc.as_texture().ArraySize;
						for (uint a = 0; a < array_size; a++)
						{
							uint first_subresource = a * mip_levels;
							auto data = read_range(first_subresource, mip_levels);

							GPUBinaryData<false> binary(GPUBinaryData<false>::Texture{ first_subresource, mip_levels }, data, get_device());

							ar& NVP(binary);
						}

					


				}
			}
		};

	}


}

