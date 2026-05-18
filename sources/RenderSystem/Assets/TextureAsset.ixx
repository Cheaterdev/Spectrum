export module Graphics:TextureAsset;
import <RenderSystem.h>;
import :Asset;
import Core;
import HAL;

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

	public:


		using ptr = s_ptr<TextureAsset>;
		using ref = AssetReference<TextureAsset>;

		TextureAsset(std::filesystem::path file_name);
		TextureAsset();

		HAL::Texture::ptr get_texture();
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

			texture->resource->set_name(convert(name));

			if (!texture)
				texture = HAL::Texture::null;
		}

	};

}