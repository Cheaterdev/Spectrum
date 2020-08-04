#include "pch.h"



MipMapGenerator::MipMapGenerator()
{
	std::lock_guard<std::mutex> g(m);


	Render::ComputePipelineStateDesc dcdesc;
	Render::RootSignatureDesc root_desc;

	dcdesc.root_signature = get_Signature(Layouts::DefaultLayout);
	dcdesc.shader = Render::compute_shader::get_resource({ "shaders\\GenerateMips.hlsl", "CS", 0, { } });

	for (int i = 0; i < 4; i++)
	{
		dcdesc.shader = Render::compute_shader::get_resource({ "shaders\\GenerateMips.hlsl", "CS", 0, { D3D::shader_macro({ std::string("NON_POWER_OF_TWO"), std::to_string(i) }) } });
		linear[i].reset(new Render::ComputePipelineState(dcdesc));
	}

	for (int i = 0; i < 4; i++)
	{
		dcdesc.shader = Render::compute_shader::get_resource({ "shaders\\GenerateMips.hlsl", "CS", 0, {
				D3D::shader_macro({ std::string("NON_POWER_OF_TWO"), std::to_string(i) }),
				D3D::shader_macro({ std::string("CONVERT_TO_SRGB"), "1" })
			}
			});
		gamma[i].reset(new Render::ComputePipelineState(dcdesc));
	}

	
	{
		dcdesc.shader = Render::compute_shader::get_resource({ "shaders\\downsample_depth.hlsl", "CS", 0 });
		state_downsample_depth.reset(new Render::ComputePipelineState(dcdesc));
	}


	{
		Render::PipelineStateDesc state_desc;
		state_desc.root_signature = get_Signature(Layouts::DefaultLayout);
		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\copy_texture.hlsl", "PS", 0,{} });
		state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\copy_texture.hlsl", "VS", 0,{} });
		state_desc.rtv.rtv_formats = { DXGI_FORMAT_R8G8B8A8_UNORM };
		state_desc.blend.render_target[0].enabled = false;
		state_desc.rtv.enable_stencil = false;
		state_desc.rtv.enable_depth = false;
		copy_texture_state.reset(new Render::PipelineState(state_desc));

	}


	{

		Render::PipelineStateDesc desc;
		desc.root_signature = get_Signature(Layouts::DefaultLayout);
		desc.vertex = Render::vertex_shader::get_resource({ "shaders/depth_render.hlsl", "VS", 0,{} });
		desc.pixel = Render::pixel_shader::get_resource({ "shaders/depth_render.hlsl", "PS", 0,{} });
		desc.rtv.rtv_formats = {};
		desc.rtv.enable_depth = true;
		desc.rtv.enable_depth_write = true;
		desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		desc.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
		render_depth_state = Render::PipelineStateCache::get().get_cache(desc);
	}


	{
		Render::PipelineStateDesc state_desc;
		state_desc.root_signature = get_Signature(Layouts::DefaultLayout);

		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\gbuffer_quality.hlsl", "PS", 0,{} });
		state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\gbuffer_quality.hlsl", "VS", 0,{} });
		state_desc.rtv.rtv_formats = { DXGI_FORMAT_R8G8_UNORM };
		state_desc.blend.render_target[0].enabled = false;
		state_desc.rtv.enable_stencil = false;
		state_desc.rtv.enable_depth = false;
		quality_buffer_state.reset(new Render::PipelineState(state_desc));



		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\gbuffer_quality.hlsl", "PS_STENCIL", 0,{} });
		state_desc.rtv.enable_stencil = true;
		state_desc.rtv.enable_depth = false;
		state_desc.rtv.stencil_desc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		state_desc.rtv.stencil_desc.StencilPassOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE;
		state_desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		state_desc.rtv.stencil_read_mask = 1;
		state_desc.rtv.stencil_write_mask = 1;
		quality_buffer_stencil_state.reset(new Render::PipelineState(state_desc));



		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\gbuffer_quality.hlsl", "PS_STENCIL_REFL", 0,{} });
		state_desc.rtv.stencil_read_mask = 2;
		state_desc.rtv.stencil_write_mask = 2;
		quality_buffer_refl_stencil_state.reset(new Render::PipelineState(state_desc));
	}


}



void MipMapGenerator::generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex)
{
	generate(compute_context, tex, tex->texture_2d());
}
void MipMapGenerator::generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, CubemapView::ptr view)
{
	for (int i = 0; i < 6; i++) generate(compute_context, tex, view->face(i));
}

void MipMapGenerator::generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Texture2DView::ptr view)
{

	std::lock_guard<std::mutex> g(m);
	auto timer = compute_context.get_base().start(L"downsampling");

	compute_context.set_signature(linear[0]->desc.root_signature);
	uint32_t maps = tex->get_desc().MipLevels - 1;
	uint32_t prev = 0;

	for (uint32_t TopMip = 0; TopMip < maps;)
	{
		uint32_t SrcWidth = uint32_t(tex->get_desc().Width >> TopMip);
		uint32_t SrcHeight = uint32_t(tex->get_desc().Height >> TopMip);
		uint32_t DstWidth = SrcWidth >> 1;
		uint32_t DstHeight = SrcHeight >> 1;
		uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;

		if (DirectX::IsSRGB(tex->get_desc().Format))
			compute_context.set_pipeline(gamma[NonPowerOfTwo]);
		else
			compute_context.set_pipeline(linear[NonPowerOfTwo]);

		uint32_t AdditionalMips;
		_BitScanForward((unsigned long*)& AdditionalMips, DstWidth | DstHeight);
		uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);

		if (TopMip + NumMips > maps)
			NumMips = maps - TopMip;

		// These are clamped to 1 after computing additional mips because clamped
		// dimensions should not limit us from downsampling multiple times.  (E.g.
		// 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
		if (DstWidth == 0)
			DstWidth = 1;

		if (DstHeight == 0)
			DstHeight = 1;

		Slots::MipMapping data;
		data.GetSrcMipLevel() = TopMip;
		data.GetNumMipLevels() = NumMips;
		data.GetTexelSize() = { 1.0f / DstWidth, 1.0f / DstHeight };

		for (uint32_t i = 0; i < NumMips; i++)
		{
			data.GetOutMip()[i] = view->get_uav(TopMip + 1 + i);
		}
		data.GetSrcMip() = view->get_srv(TopMip);

		data.set(compute_context,false);
		compute_context.use_dynamic = false;
		compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));	compute_context.use_dynamic = true;

		prev = TopMip;
		TopMip += NumMips;
	}
}

void MipMapGenerator::downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr& tex, Render::Texture::ptr& to)
{
	compute_context.set_pipeline(state_downsample_depth);

	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex->texture_2d()->get_static_srv();
	data.GetTargetTex() = to->texture_2d()->get_static_uav();
	data.set(compute_context);
	compute_context.use_dynamic = false;
	compute_context.dispach(ivec2(tex->get_desc().Width, tex->get_desc().Height), ivec2(8, 8));
	compute_context.use_dynamic = true;

}

void MipMapGenerator::downsample_depth(Render::ComputeContext& compute_context, Render::TextureView& tex, Render::TextureView& to){
	compute_context.set_pipeline(state_downsample_depth);


	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex.get_srv();
	data.GetTargetTex() = to.get_uav();
	data.set(compute_context);
	compute_context.use_dynamic = false;
	compute_context.dispach(ivec2(tex.get_size()), ivec2(8, 8));
	compute_context.use_dynamic = true;

}

void MipMapGenerator::generate_quality(Render::GraphicsContext& list, camera* cam, GBuffer& buffer, TextureView tempColor)
{
	list.set_signature(get_Signature(Layouts::DefaultLayout));

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(tempColor.get_viewport());
	list.set_scissor(tempColor.get_scissor());


	{
		{
			Slots::GBuffer gbuffer;
		buffer.SetTable(gbuffer);
		gbuffer.set(list);
		}
		list.set_pipeline(quality_buffer_state);
		list.set_rtv(1, tempColor.get_rtv(),Handle());
		list.draw(4);
	}


	
	{

		Slots::GBufferQuality quality;
		quality.GetRef() = tempColor.get_srv();
		quality.set(list);

		list.get_base().clear_stencil(buffer.quality.get_dsv());
		list.set_pipeline(quality_buffer_stencil_state);


		list.set_rtv(0, Handle(), buffer.quality.get_dsv());

		list.get_native_list()->OMSetStencilRef(1);
		list.draw(4);

		list.set_pipeline(quality_buffer_refl_stencil_state);
		list.get_native_list()->OMSetStencilRef(2);
		list.draw(4);
	}
}


void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr& to, Render::Texture::ptr& from)
{
	list.set_pipeline(copy_texture_state);
	//MipMapSignature<Signature> shader_data(&list);

	auto& view = to->texture_2d();

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());


	Slots::CopyTexture data;
	data.GetSrcTex()  = from->texture_2d()->get_srv();
	data.set(list);

	list.set_rtv(1, view->get_rtv(), Handle());	
	list.use_dynamic = false;
	list.draw(4);
	list.use_dynamic = true;
}


void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr& to, Render::TextureView from)
{
	list.set_pipeline(copy_texture_state);

	auto& view = to->texture_2d();

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());

	Slots::CopyTexture data;
	data.GetSrcTex() = from.get_srv();
	data.set(list);

	list.set_rtv(1, view->get_rtv(), Handle());	
	list.use_dynamic = false;
	list.draw(4);
	list.use_dynamic = true;
}



void MipMapGenerator::render_texture_2d_slow(Render::GraphicsContext& list, Render::TextureView to, Render::TextureView from)
{

	list.set_pipeline(copy_texture_state);
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Slots::CopyTexture data;
	data.GetSrcTex() = from.get_srv();
	data.set(list);

	list.set_rtv(1,to.get_rtv(), Handle());
	list.use_dynamic = false;
	list.draw(4);
	list.use_dynamic = true;
}



void MipMapGenerator::write_to_depth(Render::GraphicsContext& list, Render::TextureView from, Render::TextureView to)
{
	list.set_pipeline(render_depth_state);
	Slots::CopyTexture data;
	data.GetSrcTex() = from.get_srv();
	data.set(list);

	list.set_viewport(to.get_viewport());
	list.set_scissor(to.get_scissor());

	list.set_rtv(0, Handle(), to.get_dsv());

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.draw(4);
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}