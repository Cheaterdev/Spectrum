#pragma once
#include "DX12/Texture.h"
#include "Assets/TextureAsset.h"

class BRDF: public TextureAsset
{
	DX12::Texture::ptr brdf_texture;

public:

	void create_new();


private:

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int);
};



BOOST_CLASS_EXPORT_KEY(BRDF);