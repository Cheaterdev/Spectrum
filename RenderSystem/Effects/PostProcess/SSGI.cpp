#include "pch.h"
#include "SSR.h"


SSGI::SSGI(G_Buffer& buffer)
{
	this->buffer = &buffer;

	buffer.size.register_change(this, [this](const ivec2& size) {

		textures.resize(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, { size.x, size.y });
			reflect_tex = textures.first();
//		reflect_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	});


	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 4);
	root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::PIXEL);
	root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 4, 1);
	root_desc[3] = Render::DescriptorConstants(1, 1, Render::ShaderVisibility::PIXEL);
	root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 5, 3);
	//	root_desc[6] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 6, 1);
	root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearClampDesc);
	root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
	Render::PipelineStateDesc desc;
	desc.root_signature.reset(new Render::RootSignature(root_desc));
	desc.blend.render_target[0].enabled = false;
	desc.blend.render_target[0].source = D3D12_BLEND_BLEND_FACTOR;
	desc.blend.render_target[0].dest = D3D12_BLEND_INV_BLEND_FACTOR;
	desc.blend.render_target[0].source_alpha = D3D12_BLEND_BLEND_FACTOR;
	desc.blend.render_target[0].dest_alpha = D3D12_BLEND_INV_BLEND_FACTOR;
	desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\SSGI.hlsl", "PS", 0,{} });
	desc.vertex = Render::vertex_shader::get_resource({ "shaders\\SSGI.hlsl", "VS", 0,{} });
	state.reset(new  Render::PipelineState(desc));


	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\SSGI.hlsl", "PS_LAST", 0,{} });
	desc.blend.render_target[0].enabled = true;
	desc.blend.render_target[0].source = D3D12_BLEND_ONE;
	desc.blend.render_target[0].dest = D3D12_BLEND_ONE;
	state_last.reset(new  Render::PipelineState(desc));
}

void SSGI::process(MeshRenderContext::ptr& context, G_Buffer& buffer, CubemapArrayView::ptr &cube, bool clear , int repeat_count)
{//
	auto timer = context->list->start(L"reflection");
	auto& list = context->list->get_graphics();
	MipMapGenerator::get().generate(context->list->get_compute(), buffer.result_tex.first());
	context->list->transition(buffer.result_tex.first(), Render::ResourceState::PIXEL_SHADER_RESOURCE);

	if(clear)
	{
			list.transition(textures.first(), Render::ResourceState::RENDER_TARGET);
			list.clear_rtv(textures.first()->texture_2d()->get_rtv());
	}
	
	reflect_tex = textures.first();
	auto v = reflect_tex->texture_2d()->get_viewport();
	list.set_viewport(v);
	list.set_scissor(reflect_tex->texture_2d()->get_scissor());
	list.set_pipeline(state);
	list.set_dynamic(0, 0, buffer.srv_table);
	list.set(1, context->cam->get_const_buffer());
	list.set_dynamic(2, 0, buffer.result_tex.first()->texture_2d()->get_srv());
	list.set_dynamic(4, 1, buffer.speed_tex->texture_2d()->get_srv());
	list.set_dynamic(4, 2, cube->get_srv());


	for (int i = 0; i < repeat_count; i++) {
		textures.swap(context->list);
		reflect_tex = textures.first();
		list.transition(reflect_tex, Render::ResourceState::RENDER_TARGET);		
		list.set_rtv(1, reflect_tex->texture_2d()->get_rtv(), Render::Handle());	
		list.set_constants(3, ((context->current_time) % 10000+ i) * 0.001f);
		list.set_dynamic(4, 0, textures.second()->texture_2d()->get_srv());

		{
			auto timer = context->list->start(L"effect");
			list.draw(4);
		}
	}
	
	list.transition(reflect_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.transition(buffer.result_tex.first(), Render::ResourceState::RENDER_TARGET);
	//   buffer.result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.set_pipeline(state_last);
	list.set_viewport(buffer.result_tex.first()->texture_2d()->get_viewport());
	list.set_scissor(buffer.result_tex.first()->texture_2d()->get_scissor());
	list.set_rtv(1, buffer.result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
	list.set_dynamic(2,0, reflect_tex->texture_2d()->get_srv());
	list.set_dynamic(0, 0, buffer.srv_table);

	{
		auto timer = context->list->start(L"blend");
		list.draw(4);
	}
}

