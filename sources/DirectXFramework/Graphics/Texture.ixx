export module Graphics:Texture;

import serialization;
import :CommandList;
import ResourceManager;

import :Device;
import :TextureData;



//using namespace HAL;

import :Types;
import :Definitions;
export
{
	namespace Graphics
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


		class Texture : public resource_manager<Texture, texure_header>, public HAL::Resource
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
			using ptr = s_ptr<Texture>;

			ivec3 get_size(int mip = 0);

			TextureView& texture_2d() { return texture_2d_view; }
			Texture3DView& texture_3d() { return texture_3d_view; }
			CubeView& cube() { return cube_view; }

			static ptr load_native(const texure_header& header, resource_file_depender& depender);

			bool is_rt();


			static const ptr null;

			Texture(D3D::Resource native, ResourceState state);
			Texture(HAL::ResourceDesc desc, ResourceState state = ResourceState::PIXEL_SHADER_RESOURCE, HeapType heap_type = HeapType::DEFAULT, std::shared_ptr<texture_data> data = nullptr);

			texture_data::ptr get_data() const;

			void upload_data(texture_data::ptr);
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
					texture_data::ptr data;
					ar& NVP(data);
					upload_data(data);
				}
			}
		};
	}


}