#pragma once


class BRDF: public TextureAsset
{
	Render::Texture::ptr brdf_texture;

	Render::ComputePipelineState::ptr state;


public:

	void create_new();
};



BOOST_CLASS_EXPORT_KEY(BRDF);