class TextureAsset;
class TextureAssetRenderer : public Singleton<TextureAssetRenderer>
{
	friend class Singleton<TextureAssetRenderer>;

	TextureAssetRenderer();
	virtual ~TextureAssetRenderer();
public:
	void render(TextureAsset* asset, Render::Texture::ptr target, Render::CommandList::ptr c);
};


class TextureAsset : public Asset
{
protected:
	Render::Texture::ptr texture;

	//  LEAK_TEST(TextureAsset)

public:


	using ptr = s_ptr<TextureAsset>;
	using ref = AssetReference<TextureAsset>;

	TextureAsset(std::filesystem::path file_name);
	TextureAsset();

	Render::Texture::ptr get_texture();
	bool compress();

	virtual Asset_Type get_type();
	virtual void update_preview(Render::Texture::ptr preview);
	virtual void try_register();
	virtual void reload_resource() override;

private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& NVP(boost::serialization::base_object<Asset>(*this));

		if (texture == Render::Texture::null)
			texture = nullptr;

		ar& NVP(texture);

		if (!texture)
			texture = Render::Texture::null;
	}

};
BOOST_CLASS_EXPORT_KEY(AssetReference<TextureAsset>);
