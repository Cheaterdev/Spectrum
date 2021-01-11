#pragma once
#include "DX12/CommandList.h"
#include "DX12/Texture.h"
#include "DX12/ResourceViews.h"
#include "Camera/Camera.h"
#include "Context/Context.h"

class MipMapGenerator: public Singleton<MipMapGenerator>
{

    public:
        MipMapGenerator();
		void generate(DX12::ComputeContext& compute_context, DX12::Texture::ptr tex/*, DX12::Texture2DView::ptr view*/);
		void generate(DX12::ComputeContext& compute_context, DX12::Texture::ptr tex, DX12::Texture2DView::ptr view);
		void generate(DX12::ComputeContext& compute_context, TextureView view);
		void generate_cube(DX12::ComputeContext& compute_context, TextureView view);
		void downsample_depth(DX12::ComputeContext& compute_context, DX12::Texture::ptr tex, DX12::Texture::ptr& to);

		void downsample_depth(DX12::ComputeContext& compute_context, DX12::TextureView& tex, DX12::TextureView& to);

		void generate_quality(DX12::GraphicsContext& context, camera* cam, GBuffer& buffer, TextureView tempColor);
		void write_to_depth(DX12::GraphicsContext& list, DX12::TextureView from, DX12::TextureView to);


		void copy_texture_2d_slow(DX12::GraphicsContext& context, DX12::Texture::ptr to, DX12::Texture::ptr from);
		void copy_texture_2d_slow(DX12::GraphicsContext& context, DX12::Texture::ptr to, DX12::TextureView from);
		void render_texture_2d_slow(DX12::GraphicsContext& context, DX12::TextureView to, DX12::TextureView from);

};