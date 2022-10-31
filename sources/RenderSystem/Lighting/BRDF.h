#pragma once
#include "Assets/TextureAsset.h"

class BRDF: public TextureAsset
{
	HAL::Texture::ptr brdf_texture;

public:

	void create_new();


private:

	SERIALIZE();
};


