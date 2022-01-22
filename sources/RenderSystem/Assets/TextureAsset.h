#pragma once
#include "Assets/Asset.h"
import CommandList;

class TextureAsset;
class TextureAssetRenderer : public Singleton<TextureAssetRenderer>
{
	friend class Singleton<TextureAssetRenderer>;

	TextureAssetRenderer();
	virtual ~TextureAssetRenderer();
public:
	void render(TextureAsset* asset, Graphics::Texture::ptr target, Graphics::CommandList::ptr c);
};


class TextureAsset : public Asset
{
protected:
	Graphics::Texture::ptr texture;

	//  LEAK_TEST(TextureAsset)

public:


	using ptr = s_ptr<TextureAsset>;
	using ref = AssetReference<TextureAsset>;

	TextureAsset(std::filesystem::path file_name);
	TextureAsset();

	Graphics::Texture::ptr get_texture();
	bool compress();

	virtual Asset_Type get_type();
	virtual void update_preview(Graphics::Texture::ptr preview);
	virtual void try_register();
	virtual void reload_resource() override;

private:
	SERIALIZE()
	{
		SAVE_PARENT(Asset);

		if (texture == Graphics::Texture::null)
			texture = nullptr;

		ar& NVP(texture);

		if (!texture)
			texture = Graphics::Texture::null;
	}

};
