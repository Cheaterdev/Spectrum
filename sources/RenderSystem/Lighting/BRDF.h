#pragma once
#include "Assets/TextureAsset.h"

class BRDF: public TextureAsset
{
	Render::Texture::ptr brdf_texture;

public:

	void create_new();


private:

	SERIALIZE();
};



BOOST_CLASS_EXPORT_KEY(BRDF);