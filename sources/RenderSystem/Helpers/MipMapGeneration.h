#pragma once
#include "Context/Context.h"

import Graphics;

class MipMapGenerator : public Singleton<MipMapGenerator>
{

public:
	MipMapGenerator();
	void generate(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex/*, HAL::Texture2DView::ptr view*/);
	void generate(HAL::ComputeContext& compute_context, HAL::TextureView view);
	void generate_cube(HAL::ComputeContext& compute_context, HAL::CubeView view);
	void downsample_depth(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex, HAL::Texture::ptr& to);

	void downsample_depth(HAL::ComputeContext& compute_context, HAL::TextureView& tex, HAL::TextureView& to);

	void generate_quality(HAL::GraphicsContext& context, camera* cam, GBuffer& buffer, HAL::TextureView tempColor);
	void write_to_depth(HAL::GraphicsContext& list, HAL::TextureView from, HAL::TextureView to);


	void copy_texture_2d_slow(HAL::GraphicsContext& context, HAL::Texture::ptr to, HAL::Texture::ptr from);
	void copy_texture_2d_slow(HAL::GraphicsContext& context, HAL::Texture::ptr to, HAL::TextureView from);
	void render_texture_2d_slow(HAL::GraphicsContext& context, HAL::TextureView to, HAL::TextureView from);

};