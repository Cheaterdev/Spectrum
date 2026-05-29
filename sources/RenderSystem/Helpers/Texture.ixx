export module Graphics:Texture;

import Core;
import HAL;

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
			Device* m_device = nullptr;

			Texture();

			Texture(void*);


			Format original_format;

			Texture2DView texture_2d_view;
			Texture3DView texture_3d_view;
			CubeView cube_view;

			void init();
		public:
			const ResourceDesc& get_desc() const;
			HAL::TextureResource::ptr resource;

			HAL::TextureResource::ptr feedback;
			using ptr = s_ptr<Texture>;

			ivec3 get_size(int mip = 0);
			Texture2DView& texture_2d();
			Texture3DView& texture_3d();
			CubeView& cube();

			static ptr load_native(const texure_header& header, resource_file_depender& depender);
			static ptr load_native(Device& device, const texure_header& header, resource_file_depender& depender);

			bool is_rt();


			static const ptr null;

			Texture(Device& device, D3D::Resource native, TextureLayout initialLayout);
			Texture(Device& device, HAL::ResourceDesc desc, TextureLayout initialLayout = TextureLayout::UNDEFINED);

			static Texture::ptr create(Device& device, HAL::texture_data::ptr& data);

			HAL::texture_data::ptr get_data() const;

			void upload_data(HAL::texture_data::ptr);
			Texture::ptr compress();

			SERIALIZE()
			{
				ar& NVP(resource);
				if constexpr (Archive::is_saving::value)
				{

				}
				else
				{
					init();
					resource->set_name(header.name.string());
				}
			}
		};
	}


}
