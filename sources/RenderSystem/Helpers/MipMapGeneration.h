#pragma once

class MipMapGenerator: public Singleton<MipMapGenerator>
{
        Render::ComputePipelineState::ptr linear[4];
        Render::ComputePipelineState::ptr gamma[4];
        Render::ComputePipelineState::ptr state_downsample_depth;

		Render::PipelineState::ptr quality_buffer_state;
		Render::PipelineState::ptr quality_buffer_stencil_state;
		Render::PipelineState::ptr quality_buffer_refl_stencil_state;


		 
		 Cache<DXGI_FORMAT, Render::PipelineState::ptr> copy_texture_state;
		Render::PipelineState::ptr render_depth_state;

        std::mutex m;
    public:
        MipMapGenerator();
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex/*, Render::Texture2DView::ptr view*/);
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Render::Texture2DView::ptr view);
		void generate(Render::ComputeContext& compute_context, TextureView view);
		void generate_cube(Render::ComputeContext& compute_context, TextureView view);
		void downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr& tex, Render::Texture::ptr& to);

		void downsample_depth(Render::ComputeContext& compute_context, Render::TextureView& tex, Render::TextureView& to);

		void generate_quality(Render::GraphicsContext& context, camera* cam, GBuffer& buffer, TextureView tempColor);
		void write_to_depth(Render::GraphicsContext& list, Render::TextureView from, Render::TextureView to);


		void copy_texture_2d_slow(Render::GraphicsContext& context, Render::Texture::ptr& to, Render::Texture::ptr& from);
		void copy_texture_2d_slow(Render::GraphicsContext& context, Render::Texture::ptr& to, Render::TextureView from);
		void render_texture_2d_slow(Render::GraphicsContext& context, Render::TextureView to, Render::TextureView from);

};