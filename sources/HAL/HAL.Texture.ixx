export module HAL:Texture;
import Core;

import :Resource;
import :ResourceViews;
import :TextureData;

//using namespace HAL;

import :Types;

export
{
	namespace HAL
	{

		struct texure_header
		{
			std::filesystem::path name;
			bool force_srgb;
			bool mips = true;
			texure_header() = default;
			virtual ~texure_header() = default;

			texure_header(std::filesystem::path name, bool force_srgb = false, bool mips = true);
			auto  operator<=>(const  texure_header& r)  const = default;
		private:
			SERIALIZE()
			{
				ar& NVP(name)& NVP(force_srgb)& NVP(mips);
			}
		};

		enum class PixelSpace : int {
			NONE,
			MAKE_LINERAR,
			MAKE_SRGB
		};


		class Texture : public resource_manager<Texture, texure_header>
		{
			friend class resource_manager<Texture, texure_header>;
		protected:

			Texture();

			Texture(void*);


			Format original_format;

			TextureView texture_2d_view;
			Texture3DView texture_3d_view;
			CubeView cube_view;

			void init();
		public:
				const ResourceDesc& get_desc() const
			{
				return resource->get_desc();
			}
			 HAL::Resource::ptr resource;
			using ptr = s_ptr<Texture>;

			ivec3 get_size(int mip = 0);

			TextureView& texture_2d() { return texture_2d_view; }
			Texture3DView& texture_3d() { return texture_3d_view; }
			CubeView& cube() { return cube_view; }

			static ptr load_native(const texure_header& header, resource_file_depender& depender);

			bool is_rt();


			static const ptr null;

			Texture(D3D::Resource native, ResourceState state);
			Texture(HAL::ResourceDesc desc, ResourceState state = ResourceState::PIXEL_SHADER_RESOURCE, HeapType heap_type = HeapType::DEFAULT);

			static Texture::ptr create(HAL::texture_data::ptr& data, HeapType heap_type = HeapType::DEFAULT);

			HAL::texture_data::ptr get_data() const;

			void upload_data(HAL::texture_data::ptr);
			Texture::ptr compress();

			SERIALIZE()
			{

				if constexpr (Archive::is_saving::value)
				{
					auto data = get_data();
					ar& NVP(data);
				}
				else
				{
					HAL::texture_data::ptr data;
					ar& NVP(data);
					upload_data(data);
				}
			}
		};
	}


}