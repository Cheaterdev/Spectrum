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

		// Off for a texture that must always be there whole (UI, anything sampled outside the main view's materials).
		bool streamable = true;

		// What the materials bind when the texture streams (get_render_texture): made from `tiles` when the asset file
		// has them, otherwise a copy of `texture` made on first use. Never saved itself.
		std::shared_ptr<TextureStreaming::Streamed> streamed;
		bool stream_checked = false;

		// A streamable texture is saved as its tiles (TextureStreaming::TileFile: one range of the file each, in the GPU's
		// layout) and, for a device that cannot map tiles and for anything that wants the whole texture, its texels
		// (HAL::texture_data); the main archive then holds no texture and a load puts nothing but the roots in memory.
		// Partitions: "texflags" always, "textiles" and "texdata" when tiled. A file without "texflags" is an older asset:
		// the whole texture in the main archive.
		std::shared_ptr<TextureStreaming::TileFile> tiles;
		HAL::texture_data::ptr saving_data;
		bool saving_tiled = false;
		std::mutex texture_mutex;

		struct Flags
		{
			bool streamable = true;
			bool tiled = false;

			SERIALIZE()
			{
				ar& NVP(streamable);
				ar& NVP(tiled);
			}
		};

		void reset_streamed();
		bool can_stream(const HAL::Texture::ptr& candidate) const;

	public:


		using ptr = s_ptr<TextureAsset>;
		using ref = AssetReference<TextureAsset>;

		TextureAsset(std::filesystem::path file_name);
		TextureAsset(HAL::texture_data::ptr data, std::wstring name);
		TextureAsset();

		// The whole texture. For an asset loaded from its tiles this reads the texels from the file the first time (and keeps
		// them in video memory from then on), so prefer get_render_texture() wherever a streamed one will do.
		HAL::Texture::ptr get_texture();

		// What the materials bind: a streamed texture when the asset is streamable and large, so only the mips the draws
		// sample take memory from the streaming pool; any other texture as it is. Use get_texture() for everything that
		// wants all the texels.
		HAL::Texture::ptr get_render_texture();
		bool compress();

		bool is_streamable() const { return streamable; }
		void set_streamable(bool value);

		virtual Asset_Type get_type();
		virtual void update_preview(HAL::Texture::ptr preview);
		virtual void try_register();
		virtual void reload_resource() override;

		virtual void before_save() override;
		virtual void save_partitions(FileDataStorage& storage) override;
		virtual void after_save(FileDataStorage& storage) override;
		virtual void load_partitions(FileDataStorage& storage, UniversalContext* context) override;

	private:
		SERIALIZE()
		{
			SAVE_PARENT(Asset);

			// A texture saved as its tiles is not saved here.
			HAL::Texture::ptr kept = texture;
			if (texture == HAL::Texture::null || saving_tiled)
				texture = nullptr;

			ar& NVP(texture);
			cereal::wide_serialize(ar, name);

			if constexpr (Archive::is_loading::value)
			{
				if (!texture)
					texture = HAL::Texture::null;

				if (texture->resource)
					texture->resource->set_name(convert(name));
			}
			else
			{
				texture = kept;
			}
		}

	};

}