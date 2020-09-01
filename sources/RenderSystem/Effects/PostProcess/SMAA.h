#pragma once
class SMAA :Events::prop_handler, public FrameGraphGenerator
{
	Render::PipelineState::ptr state_edge_detect;
	Render::PipelineState::ptr state_blend_weight;
	Render::PipelineState::ptr state_neighborhood_blending;
	Render::PipelineState::ptr state_resolve;


	Render::Texture::ptr area_tex;
	Render::Texture::ptr search_tex;
public:
	using ptr = std::shared_ptr<SMAA>;
	SMAA();
	virtual void generate(FrameGraph& graph) override;

};
