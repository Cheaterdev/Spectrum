#include "pch.h"

SMAA::SMAA(G_Buffer& buffer)
{
	this->buffer = &buffer;
	
	area_tex = Render::Texture::get_resource({ "textures\\AreaTex.dds", false, false });
	search_tex = Render::Texture::get_resource({ "textures\\SearchTex.dds", false, false });
	//     D3D::shader_macro HLSL_VERSION("SMAA_HLSL_4_1");
	Render::PipelineStateDesc desc;
	{
		Render::RootSignatureDesc root_desc;
		root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 5);
		root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
		//root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::PIXEL, 0, 2);
		root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 5, 1);
		root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 6, 1);
		//  root_desc[3] = Render::DescriptorConstants(1, 3, Render::ShaderVisibility::PIXEL);
		root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
		root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
		desc.root_signature.reset(new Render::RootSignature(root_desc));
	}
	desc.blend.render_target[0].enabled = false;
	desc.vertex = Render::vertex_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_SMAAEdgeDetectionVS", 0,{} });
	desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM };
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_SMAALumaEdgeDetectionPS", 0,{} });
	state_edge_detect.reset(new  Render::PipelineState(desc));
	desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM };
	desc.vertex = Render::vertex_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_SMAABlendingWeightCalculationVS", 0,{} });
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_SMAABlendingWeightCalculationPS", 0,{} });
	state_blend_weight.reset(new  Render::PipelineState(desc));
	desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
	desc.vertex = Render::vertex_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_SMAANeighborhoodBlendingVS", 0,{} });
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_SMAANeighborhoodBlendingPS", 0,{} });
	state_neighborhood_blending.reset(new  Render::PipelineState(desc));
	/*      desc.vertex = Render::vertex_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_ResolveVS", 0, {} });
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\SMAA.hlsl", "DX10_ResolvePS", 0, {} });
	state_resolve.reset(new  Render::PipelineState(desc));*/
	srvs_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(5);
	srvs_table[0] = area_tex->texture_2d()->srv();
	srvs_table[1] = search_tex->texture_2d()->srv();
	//  const_buffer = Render::GPUBuffer::create_const_buffer<float4>();

	buffer.size.register_change(this, [this](const ivec2& size) {
		this->size = size;
		edges_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		blend_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		srvs_table[2] = edges_tex->texture_2d()->srv();
		srvs_table[3] = blend_tex->texture_2d()->srv();
		/* srvs_table[4] = buffer.light_tex2->texture_2d()->srv(0);*/
		srvs_table[4] = this->buffer->depth_tex_mips->texture_2d()->srv(0);
		//      const_buffer->set_data(float4(1.0f / size.x, 1.0f / size.y, size));
	});
}

void SMAA::process(MeshRenderContext::ptr& context)
{
	auto timer = context->list->start(L"smaa");
	auto& list = context->list->get_graphics();
	buffer->result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	/*  list.transition(buffer->light_tex.get(), Render::ResourceState::RENDER_TARGET);*/
	//	list.assume_state(edges_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
	//	list.assume_state(blend_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);

	list.transition(edges_tex.get(), Render::ResourceState::RENDER_TARGET);
	list.transition(blend_tex.get(), Render::ResourceState::RENDER_TARGET);

	list.flush_transitions();
	const float clearColor[] = { 0, 0, 0, 0 };
	list.get_native_list()->ClearRenderTargetView(edges_tex->texture_2d()->get_rtv().cpu, clearColor, 0, nullptr);
	list.get_native_list()->ClearRenderTargetView(blend_tex->texture_2d()->get_rtv().cpu, clearColor, 0, nullptr);
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_pipeline(state_edge_detect);
	list.set_viewport(edges_tex->texture_2d()->get_viewport(0));
	list.set_scissor(edges_tex->texture_2d()->get_scissor(0));
	list.set_rtv(1, edges_tex->texture_2d()->get_rtv(0), Render::Handle());
	list.set_dynamic(0, 0, srvs_table);
	struct buff
	{
		float4 offsets;
		float4 size;

	} b;
	b.size = float4(1.0f / size.x, 1.0f / size.y, size);
	b.offsets = vec4(context->screen_subsample,0,0);// temporal.get_current_subsample();
	list.set_const_buffer(1, b);
	//     list.set(2, Render::DescriptorHeapManager::get().get_default_samplers());
	list.set_dynamic(2, 0, buffer->result_tex.second()->texture_2d()->get_srv());
	list.draw(4);
	list.set_pipeline(state_blend_weight);
	list.set_rtv(1, blend_tex->texture_2d()->get_rtv(0), Render::Handle());
	list.draw(4);
	list.transition(edges_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.transition(blend_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.set_pipeline(state_neighborhood_blending);
	list.set_rtv(1, buffer->result_tex.first()->texture_2d()->get_rtv(0), Render::Handle());
	list.draw(4);
	//resolving
	/*    {
	list.set_pipeline(state_resolve);
	buffer->result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.set(2, buffer->result_tex.second()->texture_2d()->get_srv());
	temporal.set(context->list, 3);
	list.set_rtv(1, buffer->result_tex.first()->texture_2d()->get_rtv(0), Render::Handle());
	list.draw(4);
	}*/
	//	list.transition(buffer->light_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
}
