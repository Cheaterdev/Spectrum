export module Graphics:MipMapGenerator;
import HAL;

import :Camera;
import FrameGraph;

import :Context;

using namespace FrameGraph;
export class MipMapGenerator : public Singleton<MipMapGenerator>
{

public:
	MipMapGenerator() = default;
//	void generate(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex/*, HAL::Texture2DView::ptr view*/);
	void generate(FrameContext& compute_context, HAL::TextureView view);
	void generate_cube(FrameContext& compute_context, HAL::CubeView view);
	void downsample_depth(FrameContext& compute_context, HAL::Texture::ptr tex, HAL::Texture::ptr& to);

	void downsample_depth(FrameContext& compute_context, HAL::TextureView& tex, HAL::TextureView& to);

	void generate_quality(FrameContext& context, camera* cam, GBuffer& buffer, HAL::TextureView tempColor);
	void write_to_depth(FrameContext& list, HAL::TextureView from, HAL::TextureView to);


//	void copy_texture_2d_slow(HAL::GraphicsContext& context, HAL::Texture::ptr to, HAL::Texture::ptr from);
	void copy_texture_2d_slow(FrameContext& context, HAL::Texture::ptr to, HAL::TextureView from);
	void render_texture_2d_slow(FrameContext& context, HAL::TextureView to, HAL::TextureView from);

};