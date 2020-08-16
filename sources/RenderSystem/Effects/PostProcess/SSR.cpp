#include "pch.h"

#ifdef WTF
#include "SSR.h"

template <class T>
struct ReflectionSignature : public T
{
	using T::T;
	/*
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 4);
	root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::PIXEL);
	root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 4, 3);
	root_desc[3] = Render::DescriptorConstants(1, 1, Render::ShaderVisibility::PIXEL);
	root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0,2,2);
	root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL,7,1);

*/
	typename T::template Table			<0, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 4>g_buffer = this;
	typename T::template ConstBuffer	<1, Render::ShaderVisibility::PIXEL, 0>									camera_data = this;
	typename T::template Table			<2, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 4, 3>color_tex = this;
	typename T::template Constants		<3, Render::ShaderVisibility::PIXEL, 1, 1>								constants = this;
	typename T::template Table			<4, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 2,2>motion = this;
	typename T::template Table			<5, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 7, 1>	depth_mips = this;


	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearBorderDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointBorderDesc, this };


};

ReflectionEffectPixel::ReflectionEffectPixel(G_Buffer& buffer)
{
	this->buffer = &buffer;

	buffer.size.register_change(this, [this](const ivec2& _size) {

		auto size = ivec2(this->buffer->depth_tex_mips_pow2->get_size().xy);
		reflect_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	//	reflect_tex_dilated.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		reflect_tex_dilated.resize(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size);
	
	});

	Render::PipelineStateDesc desc;
	desc.root_signature = ReflectionSignature<SignatureCreator>().create_root();// .reset(new Render::RootSignature(root_desc));
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
	auto& list =* context->list;
	auto& graphics = context->list->get_graphics();

	MipMapGenerator::get().generate(context->list->get_compute(), buffer.result_tex.second());

	list.transition(reflect_tex, Render::ResourceState::RENDER_TARGET);

	//list.transition(buffer.result_tex.second(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
	ReflectionSignature<Signature> shader_data(&graphics);


	graphics.set_viewport(reflect_tex->texture_2d()->get_viewport());
	graphics.set_scissor(reflect_tex->texture_2d()->get_scissor());
	graphics.set_rtv(1, reflect_tex->texture_2d()->get_rtv(), Render::Handle());
	graphics.set_pipeline(state);
	shader_data.g_buffer[0]= buffer.srv_table;

	shader_data.depth_mips[0]=buffer.depth_tex_mips_pow2->texture_2d()->get_srv();

	shader_data.camera_data= context->cam->get_const_buffer();
	shader_data.color_tex[0]= buffer.result_tex.second()->texture_2d()->get_srv();
	shader_data.constants.set(((context->current_time) % 10000) * 0.001f);
	//if(cubemap)
//	list.set(4, cubemap->get_srv());
	{
		auto timer = context->list->start(L"effect");
		graphics.draw(4);
	}
	
	list.transition(reflect_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	
//	reflect_tex_dilated.reset(context->list);
	list.transition(reflect_tex_dilated.first(), Render::ResourceState::RENDER_TARGET);
	list.transition(reflect_tex_dilated.second(), Render::ResourceState::PIXEL_SHADER_RESOURCE);


	graphics.set_rtv(1, reflect_tex_dilated.first()->texture_2d()->get_rtv(), Render::Handle());


	graphics.set_viewport(reflect_tex_dilated.first()->texture_2d()->get_viewport());
	graphics.set_scissor(reflect_tex_dilated.first()->texture_2d()->get_scissor());



	graphics.set_pipeline(state_dilate);
	shader_data.color_tex[ 0]= reflect_tex->texture_2d()->get_srv();
	shader_data.color_tex[1]= buffer.result_tex.second()->texture_2d()->get_srv();
	shader_data.color_tex[2] =reflect_tex_dilated.second()->texture_2d()->get_srv();
	shader_data.motion[0] = buffer.speed_tex->texture_2d()->get_srv();
	{
		auto timer = context->list->start(L"dilate");
		graphics.draw(4);
	}


	reflect_tex_dilated.swap(context->list);

	//list.transition(reflect_tex_dilated.second(), Render::ResourceState::PIXEL_SHADER_RESOURCE);

//	list.transition(reflect_tex_dilated.first(), Render::ResourceState::PIXEL_SHADER_RESOURCE);

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
}/*

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
	list.set_const_buffer(2, context->cam->get_const_buffer());
	//   list.set(3, Render::DescriptorHeapManager::get().get_default_samplers());
	list.set_table(3, buffer.result_tex.second()->texture_2d()->get_srv());
	list.set_table(4, cubemap->cubemap()->get_srv());
	list.set_table(5, ao->texture_2d()->get_srv());
	list.dispach(*buffer.size);
}
*/

#endif