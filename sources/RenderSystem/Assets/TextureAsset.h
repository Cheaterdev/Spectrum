#pragma once;
#include "Asset.h"
class TextureAsset;
class TextureAssetRenderer : public Singleton<TextureAssetRenderer>
{
	friend class Singleton<TextureAssetRenderer>;

	TextureAssetRenderer();
	virtual ~TextureAssetRenderer();
public:
	void render(TextureAsset* asset, DX12::Texture::ptr target, DX12::CommandList::ptr c);
};


class TextureAsset : public Asset
{
protected:
	DX12::Texture::ptr texture;

	//  LEAK_TEST(TextureAsset)

public:


	using ptr = s_ptr<TextureAsset>;
	using ref = AssetReference<TextureAsset>;

	TextureAsset(std::filesystem::path file_name);
	TextureAsset();

	DX12::Texture::ptr get_texture();
	bool compress();

	virtual Asset_Type get_type();
	virtual void update_preview(DX12::Texture::ptr preview);
	virtual void try_register();
	virtual void reload_resource() override;

private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& NVP(boost::serialization::base_object<Asset>(*this));

		if (texture == DX12::Texture::null)
			texture = nullptr;

		ar& NVP(texture);

		if (!texture)
			texture = DX12::Texture::null;
	}

};
BOOST_CLASS_EXPORT_KEY(AssetReference<TextureAsset>);
