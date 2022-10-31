#pragma once
#include "Camera/Camera.h"
#include "Context/Context.h"

class MipMapGenerator : public Singleton<MipMapGenerator>
{

public:
	MipMapGenerator();
	void generate(HAL::ComputeContext& compute_context, Graphics::Texture::ptr tex/*, Graphics::Texture2DView::ptr view*/);
	void generate(HAL::ComputeContext& compute_context, Graphics::TextureView view);
	void generate_cube(HAL::ComputeContext& compute_context, Graphics::CubeView view);
	void downsample_depth(HAL::ComputeContext& compute_context, Graphics::Texture::ptr tex, Graphics::Texture::ptr& to);

	void downsample_depth(HAL::ComputeContext& compute_context, Graphics::TextureView& tex, Graphics::TextureView& to);

	void generate_quality(HAL::GraphicsContext& context, camera* cam, GBuffer& buffer, Graphics::TextureView tempColor);
	void write_to_depth(HAL::GraphicsContext& list, Graphics::TextureView from, Graphics::TextureView to);


	void copy_texture_2d_slow(HAL::GraphicsContext& context, Graphics::Texture::ptr to, Graphics::Texture::ptr from);
	void copy_texture_2d_slow(HAL::GraphicsContext& context, Graphics::Texture::ptr to, Graphics::TextureView from);
	void render_texture_2d_slow(HAL::GraphicsContext& context, Graphics::TextureView to, Graphics::TextureView from);

};