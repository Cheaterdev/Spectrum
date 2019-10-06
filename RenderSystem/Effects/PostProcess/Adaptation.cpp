#include "pch.h"

template <class T>
struct HDRSignature : public T
{
	using T::T;
	/*
		root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 2);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 2);
	root_desc[2] = Render::DescriptorConstants(0, 6, Render::ShaderVisibility::ALL);
	root_desc.set_sampler(0, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerLinearClampDesc);

*/
	typename T::template Table			<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 2>source = this;
	typename T::template Table			<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 2>target = this;
	typename T::template Constants		<2, Render::ShaderVisibility::ALL, 0, 6>							constants = this;


	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearClampDesc, this };

};

HDRAdaptation::HDRAdaptation(G_Buffer & buffer)
{
	this->buffer = &buffer;
	tex_luma = std::make_shared<Texture>(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UINT, 640, 384, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE);



	Render::ComputePipelineStateDesc desc;
	desc.root_signature = HDRSignature<SignatureCreator>().create_root();// .reset(new Render::RootSignature(root_desc));

	desc.shader = Render::compute_shader::get_resource({ "shaders\\HDR_Luma_extract.hlsl", "CS", 0,{} });
	luma_state.reset(new  Render::ComputePipelineState(desc));

	desc.shader = Render::compute_shader::get_resource({ "shaders\\HDR_tonemap.hlsl", "CS", 0,{} });
	hdr_state.reset(new  Render::ComputePipelineState(desc));

	desc.shader = Render::compute_shader::get_resource({ "shaders\\HDR_histo.hlsl", "CS", 0,{} });
	histo_state.reset(new  Render::ComputePipelineState(desc));

	desc.shader = Render::compute_shader::get_resource({ "shaders\\HDR_exp.hlsl", "CS", 0,{} });
	exp_state.reset(new  Render::ComputePipelineState(desc));

	desc.shader = Render::compute_shader::get_resource({ "shaders\\HDR_tonemap.hlsl", "HISTO_CS", 0,{} });

	draw_histo_state.reset(new  Render::ComputePipelineState(desc));


	buf_exp = std::make_shared<Render::StructuredBuffer<float>>(8, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	buf_histogram = std::make_shared<Render::ByteBuffer>(256 * 4);



}

void HDRAdaptation::generate_luma(MeshRenderContext::ptr & context)
{
	auto timer = context->list->start(L"generate_luma");

	auto& list = *context->list;
	auto & compute = context->list->get_compute();

	list.transition(tex_luma, Render::ResourceState::UNORDERED_ACCESS);
	list.transition(buffer->result_tex.first(), Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

	compute.set_pipeline(luma_state);
	HDRSignature<Signature> shader_data(&compute);


	shader_data.constants.set(1.0f / tex_luma->get_size().x, 1.0f / tex_luma->get_size().y);
	shader_data.target[0] = tex_luma->texture_2d()->get_uav();
	shader_data.source[0] = buffer->result_tex.first()->texture_2d()->get_srv();

	compute.dispach(ivec2(tex_luma->get_size().xy));
}

void HDRAdaptation::process_hdr(MeshRenderContext::ptr & context)
{
	auto timer = context->list->start(L"process_hdr");

	auto & compute = context->list->get_compute();
	auto& list = *context->list;


	list.transition(tex_luma, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.transition(buffer->result_tex.first(), Render::ResourceState::UNORDERED_ACCESS);

	if (!inited)
	{
		inited = true;

		std::vector<float> data = { Exposure, 1.0f / Exposure, Exposure, 0.0f, kInitialMinLog, kInitialMaxLog, kInitialMaxLog - kInitialMinLog, 1.0f / (kInitialMaxLog - kInitialMinLog) };


		buf_exp->set_data(context->list, 0, data);
		list.transition(buf_exp, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

	}

	compute.set_pipeline(hdr_state);

	HDRSignature<Signature> shader_data(&compute);

	float ToeStrength = g_ToeStrength < 1e-6f ? 1e32f : 1.0f / g_ToeStrength;

	shader_data.constants.set(float(tex_luma->get_size().x), 1.0f / tex_luma->get_size().y, 1.0f, ToeStrength);
	shader_data.target[0]=buffer->result_tex.first()->texture_2d()->get_uav();
	shader_data.target[1] = tex_luma->texture_2d()->get_uav();

	shader_data.source[0] =  buf_exp->get_srv();

	compute.dispach(*buffer->size);

	/*
	{
	compute.set_pipeline(draw_histo_state);
	compute.set_dynamic(1, 0, buffer->result_tex.first()->texture_2d()->get_uav());
	compute.set_dynamic(0, 0, buf_histogram->get_srv());
	compute.set_dynamic(0, 1, buf_exp->get_srv());

	compute.dispach( 1,32);
	}

	*/
}

void HDRAdaptation::process_exposition(MeshRenderContext::ptr & context)
{
	auto timer = context->list->start(L"process_exposition");

	auto & compute = context->list->get_compute();
	auto& list = *context->list;

	list.transition(buf_histogram, Render::ResourceState::UNORDERED_ACCESS);
	list.transition(tex_luma, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.clear_uav(buf_histogram, buf_histogram->get_uav()[0]);

	compute.set_pipeline(histo_state);
	HDRSignature<Signature> shader_data(&compute);


	shader_data.target[0]=buf_histogram->get_uav();
	shader_data.source[0]= tex_luma->texture_2d()->get_srv();
	compute.dispach(ivec2(tex_luma->get_size().xy), ivec2(16, 384));


	//////////////////////////////////////////

	list.transition(buf_histogram, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.transition(buf_exp, Render::ResourceState::UNORDERED_ACCESS);
	compute.set_pipeline(exp_state);
	
	shader_data.target[0] = buf_exp->get_uav();
	shader_data.source[0] = buf_histogram->get_srv();


	float TargetLuminance = 0.08;
	float AdaptationRate = 0.05;
	float MinExposure = 1.0f / 64.0f;
	float MaxExposure = 64.0f;

	shader_data.constants.set(TargetLuminance, AdaptationRate, MinExposure, MaxExposure, float(tex_luma->get_size().x * tex_luma->get_size().y));
	compute.dispach();

	list.transition(buf_exp, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);



}

void HDRAdaptation::process(MeshRenderContext::ptr & context)
{
	auto timer = context->list->start(L"adaptation");

	process_hdr(context);
	generate_luma(context);
	process_exposition(context);
}
