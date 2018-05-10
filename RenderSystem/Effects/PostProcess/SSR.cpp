#include "pch.h"
#include "SSR.h"


ReflectionEffectPixel::ReflectionEffectPixel(G_Buffer& buffer)
{
	this->buffer = &buffer;

	buffer.size.register_change(this, [this](const ivec2& size) {
		reflect_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x/2, size.y/2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		reflect_tex_dilated.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x / 2, size.y / 2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));

	
	});


	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 4);
	root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::PIXEL);
	root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 4, 2);
	root_desc[3] = Render::DescriptorConstants(1, 1, Render::ShaderVisibility::PIXEL);
	//root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 5, 1);
	//	root_desc[6] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 6, 1);
	root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearClampDesc);
	root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
	Render::PipelineStateDesc desc;
	desc.root_signature.reset(new Render::RootSignature(root_desc));
	desc.blend.render_target[0].enabled = false;
	desc.blend.render_target[0].source = D3D12_BLEND_ONE;
	desc.blend.render_target[0].dest = D3D12_BLEND_ONE;
	desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\ReflectionPixel.hlsl", "PS", 0,{} });
	desc.vertex = Render::vertex_shader::get_resource({ "shaders\\ReflectionPixel.hlsl", "VS", 0,{} });
	state.reset(new  Render::PipelineState(desc));


	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\ReflectionPixelDilate.hlsl", "PS", 0,{} });
	state_dilate.reset(new  Render::PipelineState(desc));

	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\ReflectionPixel.hlsl", "PS_LAST", 0,{} });
	desc.blend.render_target[0].enabled = true;
	state_last.reset(new  Render::PipelineState(desc));
}

void ReflectionEffectPixel::process(MeshRenderContext::ptr& context, G_Buffer& buffer, Render::CubemapArrayView::ptr cubemap)
{
	auto timer = context->list->start(L"reflection");
	auto& list = context->list->get_graphics();
//	MipMapGenerator::get().generate(context->list->get_compute(), buffer.result_tex.first());

//	buffer.result_tex.swap(context->list);

	list.transition(reflect_tex, Render::ResourceState::RENDER_TARGET);
	auto v = reflect_tex->texture_2d()->get_viewport();
	//     v.Height /= 4;
	//   v.Width /= 4;
	list.set_viewport(v);
	list.set_scissor(reflect_tex->texture_2d()->get_scissor());
	list.set_rtv(1, reflect_tex->texture_2d()->get_rtv(), Render::Handle());
	list.set_pipeline(state);
	list.set_dynamic(0,0, buffer.srv_table);
	list.set(1, context->cam->get_const_buffer());
	list.set_dynamic(2,0, buffer.result_tex.first()->texture_2d()->get_srv());
	list.set_constants(3, ((context->current_time) % 10000) * 0.001f);
	//if(cubemap)
//	list.set(4, cubemap->get_srv());
	{
		auto timer = context->list->start(L"effect");
		list.draw(4);
	}

	
	list.transition(reflect_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.transition(reflect_tex_dilated, Render::ResourceState::RENDER_TARGET);

	list.set_rtv(1, reflect_tex_dilated->texture_2d()->get_rtv(), Render::Handle());


	list.set_pipeline(state_dilate);
	list.set_dynamic(2, 0, reflect_tex->texture_2d()->get_srv());
	{
		auto timer = context->list->start(L"dilate");
		list.draw(4);
	}


	list.transition(reflect_tex_dilated, Render::ResourceState::PIXEL_SHADER_RESOURCE);

//	list.transition(buffer.result_tex.first(), Render::ResourceState::RENDER_TARGET);
	//   buffer.result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
/*	list.set_pipeline(state_last);
	list.set_viewport(buffer.result_tex.first()->texture_2d()->get_viewport());
	list.set_scissor(buffer.result_tex.first()->texture_2d()->get_scissor());
	list.set_rtv(1, buffer.result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
	list.set_dynamic(2, 0, reflect_tex_dilated->texture_2d()->get_srv());

	list.set_dynamic(2, 1, buffer.result_tex.second()->texture_2d()->get_srv());
	{
		auto timer = context->list->start(L"blend");
		list.draw(4);
	}*/
}

inline ReflectionEffect::ReflectionEffect()
{
	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
	root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
	//root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 3);
	root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 4, 1);
	root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 5, 1);
	root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 6, 1);
	root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
	root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
	Render::ComputePipelineStateDesc desc;
	desc.shader = Render::compute_shader::get_resource({ "shaders\\Reflection.hlsl", "CS", 0,{} });
	desc.root_signature.reset(new Render::RootSignature(root_desc));
	state.reset(new  Render::ComputePipelineState(desc));
}

inline void ReflectionEffect::process(MeshRenderContext::ptr & context, G_Buffer & buffer, Render::Texture::ptr cubemap, Render::Texture::ptr ao)
{
	auto timer = context->list->start(L"reflection");
	auto& list = context->list->get_compute();
	MipMapGenerator::get().generate(context->list->get_compute(), buffer.result_tex.first());
	buffer.result_tex.swap(context->list, Render::ResourceState::UNORDERED_ACCESS, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.set_pipeline(state);
	list.set(0, buffer.srv_table);
	list.set_table(1, buffer.result_tex.first()->texture_2d()->get_uav());
	list.set(2, context->cam->get_const_buffer());
	//   list.set(3, Render::DescriptorHeapManager::get().get_default_samplers());
	list.set_table(3, buffer.result_tex.second()->texture_2d()->get_srv());
	list.set_table(4, cubemap->cubemap()->get_srv());
	list.set_table(5, ao->texture_2d()->get_srv());
	list.dispach(*buffer.size);
}
