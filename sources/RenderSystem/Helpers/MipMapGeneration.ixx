export module Graphics:MipMapGenerator;
import HAL;
import :Texture;

import :Camera;

import :Context;
export class MipMapGenerator : public Singleton<MipMapGenerator>
{

public:
	MipMapGenerator() = default;
//	void generate(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex/*, HAL::Texture2DView::ptr view*/);
	void generate(HAL::ComputeContext& compute_context, HAL::Texture2DView view);
	void generate_cube(HAL::ComputeContext& compute_context, HAL::CubeView view);
	void downsample_depth(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex, HAL::Texture::ptr& to);

	void downsample_depth(HAL::ComputeContext& compute_context, HAL::Texture2DView& tex, HAL::Texture2DView& to);

	// Fills mips 1..N of `view` from its already-populated mip 0, one 2x2
	// min-reduction dispatch per mip (reversed-Z conservative -- see
	// downsample_depth_mip.hlsl). Caller is responsible for mip 0's content;
	// this only builds the coarser chain on top of it.
	void build_hiz_pyramid(HAL::ComputeContext& compute_context, HAL::Texture2DView view);

	void generate_quality(HAL::GraphicsContext& context, camera* cam, GBuffer& buffer, HAL::Texture2DView tempColor);
	void write_to_depth(HAL::GraphicsContext& list, HAL::Texture2DView from, HAL::Texture2DView to);


//	void copy_texture_2d_slow(HAL::GraphicsContext& context, HAL::Texture::ptr to, HAL::Texture::ptr from);
	void copy_texture_2d_slow(HAL::GraphicsContext& context, HAL::Texture::ptr to, HAL::Texture2DView from);
	void render_texture_2d_slow(HAL::GraphicsContext& context, HAL::Texture2DView to, HAL::Texture2DView from);

};