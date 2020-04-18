#include "pch.h"
#include "GBuffer.h"

 TextureSwapper::TextureSwapper()
{
}

 void TextureSwapper::resize(DXGI_FORMAT format, ivec2 size, ResourceAllocator &allocator )
{
	data[0].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE, allocator));
	data[1].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE,allocator));

	data[0]->set_name(std::string("TextureSwapper_0"));
	data[1]->set_name(std::string("TextureSwapper_1"));
}

 void TextureSwapper::reset(Render::CommandList::ptr & list)
{
	Swapper::reset();
	list->transition(first(), Render::ResourceState::RENDER_TARGET);
	list->transition(second(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
}

 void TextureSwapper::swap(Render::CommandList::ptr & list, Render::ResourceState state1, Render::ResourceState state2)
{
	Swapper::swap();
	list->transition(first(), state1);
	list->transition(second(), state2);
}

 void G_Buffer::resize(ivec2 size)
{
	
	result_tex.resize(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size);
//	 if (albedo_tex) return;
albedo_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	normal_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, size.x, size.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	specular_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	speed_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	depth_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	depth_tex_downscaled.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x / 8, size.y / 8, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::DEPTH_WRITE));
	depth_tex_downscaled_uav.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, size.x / 8, size.y / 8, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	depth_tex_mips.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, size.x, size.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	quality_stencil.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE), Render::ResourceState::DEPTH_WRITE));
	quality_color.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM, size.x / 2, size.y / 2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::RENDER_TARGET));


	ivec2 size_pow2(1, 1);
	while (size_pow2.x < size.x)size_pow2.x <<= 1;
	while (size_pow2.y < size.y)size_pow2.y <<= 1;

	depth_tex_mips_pow2.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, size_pow2.x, size_pow2.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));

	depth_tex_mips_prev.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, size.x, size.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));

	normal_tex->debug = true;
	albedo_tex->set_name("albedo_tex");
	normal_tex->set_name("normal_tex");
	specular_tex->set_name("specular_tex");
	speed_tex->set_name("speed_tex");
	depth_tex->set_name("depth_tex");
	depth_tex_downscaled->set_name("depth_tex_downscaled");
	depth_tex_downscaled_uav->set_name("depth_tex_downscaled_uav");
	depth_tex_mips->set_name("depth_tex_mips");


	srv_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(4);
	albedo_tex->texture_2d()->place_srv(srv_table[0]);
	normal_tex->texture_2d()->place_srv(srv_table[1]);
	specular_tex->texture_2d()->place_srv(srv_table[2]);
	depth_tex_mips->texture_2d()->place_srv(srv_table[3]);
	table = RenderTargetTable({ albedo_tex, normal_tex, specular_tex, speed_tex }, depth_tex);
	table_downscaled_depth = RenderTargetTable({}, depth_tex_downscaled);
	//   table_light = RenderTargetTable({ light_tex }, depth_tex);
	vps.resize(1);
	vps[0].MinDepth = 0.0f;
	vps[0].MaxDepth = 1.0f;
	scissors.resize(1);
	vps[0].Width = static_cast<float>(size.x);
	vps[0].Height = static_cast<float>(size.y);
	vps[0].TopLeftX = 0;
	vps[0].TopLeftY = 0;
	scissors[0] = { 0, 0, size.x, size.y };
	first = true;



	quality_table = RenderTargetTable({}, quality_stencil);

	quality_color_table = RenderTargetTable({ quality_color }, nullptr);
}

 G_Buffer::G_Buffer()
{
	size.register_change(this, [this](const ivec2& size) {
		if(size.x>0)
		resize(size);
	});
}

 void G_Buffer::init(Render::RootSignature::ptr sig)
{
	Render::PipelineStateDesc desc;
	//	Render::RootSignatureDesc root_desc;
	//	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1);
	//root_desc.set_sampler(0, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerPointClampDesc);
	desc.root_signature = sig;// std::make_shared<Render::RootSignature>(root_desc);
	desc.vertex = Render::vertex_shader::get_resource({ "shaders/depth_render.hlsl", "VS", 0,{} });
	desc.pixel = Render::pixel_shader::get_resource({ "shaders/depth_render.hlsl", "PS", 0,{} });
	desc.rtv.rtv_formats = {};
	desc.rtv.enable_depth = true;
	desc.rtv.enable_depth_write = true;
	desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	desc.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
	//	desc.blend.render_target[0].enabled = true;
	render_depth_state = Render::PipelineStateCache::get().get_cache(desc);



}

 void G_Buffer::set_lights(MeshRenderContext::ptr & context)
{
	//   table_light.set(context);
	context->list->get_graphics().set_viewports(vps);
	context->list->get_graphics().set_scissors(scissors[0]);
}

 void G_Buffer::set_downsapled(Render::CommandList::ptr & list)
{
	list->transition(depth_tex_downscaled, Render::ResourceState::DEPTH_WRITE);
	table_downscaled_depth.set(list->get_graphics());
	table_downscaled_depth.set_window(list->get_graphics());

	if (first)
	{
		first = false;
		table_downscaled_depth.clear_depth(list->get_graphics());
	}
}

 void G_Buffer::set_original(MeshRenderContext::ptr & context)
{
	context->list->transition(depth_tex, Render::ResourceState::DEPTH_WRITE);
	table.set(context);
	table.set_window(context->list->get_graphics());
	//	context->list->flush_transitions();
	//	table.clear_depth(context);
}

 void G_Buffer::reset(MeshRenderContext::ptr & context)
{
	auto& list = context->list;
	result_tex.reset(list);
	result_tex.first()->texture_2d()->get_rtv().clear(*list);
	
	
}


 void G_Buffer::set(MeshRenderContext::ptr & context, bool copy_prev)
{
	auto& list = context->list;

	if(copy_prev)
		list->get_copy().copy_texture(depth_tex_mips_prev, 0, depth_tex, 0);

	//	result_tex.reset(list);
	context->list->transition(albedo_tex, Render::ResourceState::RENDER_TARGET);
	context->list->transition(normal_tex, Render::ResourceState::RENDER_TARGET);
	context->list->transition(specular_tex, Render::ResourceState::RENDER_TARGET);
	context->list->transition(speed_tex, Render::ResourceState::RENDER_TARGET);
	context->list->transition(depth_tex, Render::ResourceState::DEPTH_WRITE);
	list->flush_transitions();
	
	table.set(context);
	
	//	result_tex.first()->texture_2d()->get_rtv().clear(*list);
	// result_tex.second()->texture_2d()->get_rtv().clear(*list);
	list->get_graphics().set_viewports(vps);
	list->get_graphics().set_scissors(scissors[0]);
	context->pipeline.blend.render_target[0].enabled = true;

}

 void G_Buffer::downsample_depth(Render::CommandList::ptr & list)
{
	MipMapGenerator::get().downsample_depth(list->get_compute(), depth_tex, depth_tex_downscaled_uav);
	list->transition(depth_tex_downscaled_uav, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list->transition(depth_tex_downscaled, Render::ResourceState::DEPTH_WRITE);
	set_downsapled(list);
	list->get_graphics().set_pipeline(render_depth_state);
	((SignatureDataSetter*)&list->get_graphics())->set_dynamic(13, 0, depth_tex_downscaled_uav->texture_2d()->get_static_uav());
	list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list->get_graphics().draw(4);
	list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

 void G_Buffer::end(MeshRenderContext::ptr & context)
{
	auto& list = context->list;
	context->list->transition(albedo_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	context->list->transition(normal_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	context->list->transition(specular_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	context->list->transition(speed_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list->get_copy().copy_texture(depth_tex_mips, 0, depth_tex, 0);

	//context->list->transition(depth_tex_mips, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	//	context->list->transition(depth_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);

}

 void G_Buffer::generate_quality(MeshRenderContext::ptr & context)
{

}

 void G_Buffer::set_downsapled(MeshRenderContext::ptr & context)
{
	context->list->transition(depth_tex_downscaled, Render::ResourceState::DEPTH_WRITE);
	table_downscaled_depth.set(context);
	table_downscaled_depth.set_window(context->list->get_graphics());

	if (first)
	{
		first = false;
		table_downscaled_depth.clear_depth(context);
	}
}
