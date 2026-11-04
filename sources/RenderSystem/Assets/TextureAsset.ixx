export module Graphics:TextureAsset;

import :Asset;
import Core;
import HAL;
import :Texture;
import :TextureStreaming;

export{

	class TextureAsset;
	class TextureAssetRenderer : public Singleton<TextureAssetRenderer>
	{
		friend class Singleton<TextureAssetRenderer>;

		TextureAssetRenderer();
		virtual ~TextureAssetRenderer();
	public:
		void render(TextureAsset* asset, HAL::Texture::ptr target, HAL::CommandList::ptr c);
	};


	class TextureAsset : public Asset
	{
	protected:
		HAL::Texture::ptr texture;

		// A streamed copy of `texture` for the materials (get_render_texture), made on first use and never saved: the
		// asset on disk is `texture`.
		std::shared_ptr<TextureStreaming::Streamed> streamed;
		bool stream_checked = false;

		void reset_streamed();

	public:


		using ptr = s_ptr<TextureAsset>;
		using ref = AssetReference<TextureAsset>;

		TextureAsset(std::filesystem::path file_name);
		TextureAsset(HAL::texture_data::ptr data, std::wstring name);
		TextureAsset();

		HAL::Texture::ptr get_texture();

		// What the materials bind: a streamed copy of a large 2D texture, so only the mips the draws sample take memory
		// from the streaming pool; any other texture as it is. Use get_texture() for everything that wants all the texels.
		HAL::Texture::ptr get_render_texture();
		bool compress();

		virtual Asset_Type get_type();
		virtual void update_preview(HAL::Texture::ptr preview);
		virtual void try_register();
		virtual void reload_resource() override;

	private:
		SERIALIZE()
		{
			SAVE_PARENT(Asset);

			if (texture == HAL::Texture::null)
				texture = nullptr;

			ar& NVP(texture);
			cereal::wide_serialize(ar, name);
			
			if (!texture)
				texture = HAL::Texture::null;

			if constexpr (Archive::is_loading::value)
				if (texture->resource)
					texture->resource->set_name(convert(name));

		}

	};

}