#pragma once

class MipMapGenerator: public Singleton<MipMapGenerator>
{

    public:
        MipMapGenerator();
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex/*, Render::Texture2DView::ptr view*/);
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Render::Texture2DView::ptr view);
		void generate(Render::ComputeContext& compute_context, TextureView view);
		void generate_cube(Render::ComputeContext& compute_context, TextureView view);
		void downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Render::Texture::ptr& to);

		void downsample_depth(Render::ComputeContext& compute_context, Render::TextureView& tex, Render::TextureView& to);

		void generate_quality(Render::GraphicsContext& context, camera* cam, GBuffer& buffer, TextureView tempColor);
		void write_to_depth(Render::GraphicsContext& list, Render::TextureView from, Render::TextureView to);


		void copy_texture_2d_slow(Render::GraphicsContext& context, Render::Texture::ptr to, Render::Texture::ptr from);
		void copy_texture_2d_slow(Render::GraphicsContext& context, Render::Texture::ptr to, Render::TextureView from);
		void render_texture_2d_slow(Render::GraphicsContext& context, Render::TextureView to, Render::TextureView from);

};