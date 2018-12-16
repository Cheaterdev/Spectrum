#pragma once

class MipMapGenerator: public Singleton<MipMapGenerator>
{
        Render::ComputePipelineState::ptr linear[4];
        Render::ComputePipelineState::ptr gamma[4];
        Render::ComputePipelineState::ptr state_downsample_depth;

        Render::HandleTable samplers;

		Render::PipelineState::ptr quality_buffer_state;
		Render::PipelineState::ptr quality_buffer_stencil_state;
		Render::PipelineState::ptr quality_buffer_refl_stencil_state;


		Render::PipelineState::ptr copy_texture_state;


        std::mutex m;
    public:
        MipMapGenerator();
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex/*, Render::Texture2DView::ptr view*/);
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Render::Texture2DView::ptr view);
		void generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Render::CubemapView::ptr view);
		void downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr& tex, Render::Texture::ptr& to);


		void generate_quality(Render::GraphicsContext& context, camera* cam, G_Buffer& buffer);


		void copy_texture_2d_slow(Render::GraphicsContext& context, Render::Texture::ptr& to, Render::Texture::ptr& from);

};