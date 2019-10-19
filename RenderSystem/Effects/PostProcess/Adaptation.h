#pragma once
class HDRAdaptation
{
	Render::ComputePipelineState::ptr luma_state;
	Render::ComputePipelineState::ptr hdr_state;
	Render::ComputePipelineState::ptr histo_state;
	Render::ComputePipelineState::ptr exp_state;
	Render::ComputePipelineState::ptr draw_histo_state;

	shader_struct exp_buffer_data
	{
		float4 exp;
	float4 logs;
	};
	Render::StructuredBuffer<float>::ptr buf_exp;
	Render::ByteBuffer::ptr buf_histogram;

	G_Buffer* buffer;
	float Exposure = 2.0f;
	const float kInitialMinLog = -12.0f;
	const float kInitialMaxLog = 4.0f;
	const float g_ToeStrength = 0.01f;
	bool inited = false;
	Texture::ptr tex_luma;

public:
	using ptr = std::shared_ptr<HDRAdaptation>;
	HDRAdaptation(G_Buffer& buffer);

	void generate_luma(MeshRenderContext::ptr& context);
	void process_hdr(MeshRenderContext::ptr& context);

	void process_exposition(MeshRenderContext::ptr& context);


	void process(MeshRenderContext::ptr& context);
};