#pragma once
class SMAA :Events::prop_handler
{
	Render::PipelineState::ptr state_edge_detect;
	Render::PipelineState::ptr state_blend_weight;
	Render::PipelineState::ptr state_neighborhood_blending;
	Render::PipelineState::ptr state_resolve;


	Render::Texture::ptr area_tex;
	Render::Texture::ptr search_tex;

	//  Render::Texture::ptr edges_tex;

	Render::HandleTable srvs_table;

	// Render::GPUBuffer::ptr const_buffer;
	G_Buffer* buffer;
	float2 size;
	//	Render::CommandList
public:
	using ptr = std::shared_ptr<SMAA>;
	Render::Texture::ptr edges_tex;
	Render::Texture::ptr blend_tex;


	SMAA(G_Buffer& buffer);



	void process(MeshRenderContext::ptr& context);
};
