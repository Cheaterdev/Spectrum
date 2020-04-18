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

	compute_context.get_base().transition(tex, Render::ResourceState::UNORDERED_ACCESS);
	compute_context.set_signature(linear[0]->desc.root_signature);
	uint32_t maps = tex->get_desc().MipLevels - 1;

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

		struct CB
		{
			unsigned int SrcMipLevel;	// Texture level of source mip
			unsigned int NumMipLevels;	// Number of OutMips to write: [1, 4]
			float2 TexelSize;	// 1.0 / OutMip1.Dimensions

		} cb;
		cb.SrcMipLevel = TopMip;
		cb.NumMipLevels = NumMips;
		cb.TexelSize = { 1.0f / DstWidth, 1.0f / DstHeight };
		//	Context.SetConstants(0, TopMip, NumMips, 1.0f / DstWidth, 1.0f / DstHeight);


		Slots::MipMapping data;
		data.GetSrcMipLevel() = TopMip;
		data.GetNumMipLevels() = NumMips;
		data.GetTexelSize() = { 1.0f / DstWidth, 1.0f / DstHeight };

		for (uint32_t i = 0; i < NumMips; i++)
			data.GetOutMip()[i] = view->get_uav(TopMip + 1 + i);

		data.GetSrcMip() = view->get_srv();

		data.set(compute_context);

	
		compute_context.use_dynamic = false;
		compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));	compute_context.use_dynamic = true;
		compute_context.get_base().transition_uav(tex.get());
		TopMip += NumMips;
	}

	//   compute_context.transition(tex, after);
}

void MipMapGenerator::downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr& tex, Render::Texture::ptr& to)
{
	std::lock_guard<std::mutex> g(m);
	compute_context.get_base().transition(tex, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	compute_context.get_base().transition(to, Render::ResourceState::UNORDERED_ACCESS);
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
	std::lock_guard<std::mutex> g(m);
	compute_context.get_base().transition(tex.resource, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	compute_context.get_base().transition(to.resource, Render::ResourceState::UNORDERED_ACCESS);
	compute_context.set_pipeline(state_downsample_depth);


	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex.get_srv();
	data.GetTargetTex() = to.get_uav();
	data.set(compute_context);
	compute_context.use_dynamic = false;
	compute_context.dispach(ivec2(tex.get_size()), ivec2(8, 8));
	compute_context.use_dynamic = true;

}
/*
void MipMapGenerator::generate_quality(Render::GraphicsContext& list, camera* cam, G_Buffer& buffer)
{
	std::lock_guard<std::mutex> g(m);

	MipMapSignature<Signature> shader_data(&list);

	{
		list.set_pipeline(quality_buffer_state);
		auto& view = buffer.quality_color->texture_2d();

		list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		list.set_viewport(view->get_viewport());
		list.set_scissor(view->get_scissor());
		shader_data.constants = cam->get_const_buffer();
		shader_data.source[0]= buffer.normal_tex->texture_2d()->get_srv();
		shader_data.source[1] = buffer.depth_tex_mips->texture_2d()->get_srv();
		shader_data.source[2] = buffer.normal_tex->texture_2d()->get_srv();
		shader_data.source[3] = buffer.depth_tex->texture_2d()->get_srv();
		buffer.quality_color_table.set(list);
		list.draw(4);

	}


	{
		buffer.quality_table.clear_stencil(list, 0);

		list.set_pipeline(quality_buffer_stencil_state);
		auto& view = buffer.quality_stencil->texture_2d();
		auto& view_color = buffer.quality_color->texture_2d();

		list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		list.set_viewport(view->get_viewport());
		list.set_scissor(view->get_scissor());
		shader_data.constants = cam->get_const_buffer();

		shader_data.source[0] = buffer.normal_tex->texture_2d()->get_srv();
		shader_data.source[1] = buffer.depth_tex_mips->texture_2d()->get_srv();
		shader_data.source[2] = buffer.quality_color->texture_2d()->get_srv();
		shader_data.source[3] = buffer.quality_color->texture_2d()->get_srv();

		buffer.quality_table.set(list);


		list.get_native_list()->OMSetStencilRef(1);
		list.draw(4);

		list.set_pipeline(quality_buffer_refl_stencil_state);
		list.get_native_list()->OMSetStencilRef(2);
		list.draw(4);
	}



}

*/
void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr& to, Render::Texture::ptr& from)
{
	list.get_base().transition(to, ResourceState::RENDER_TARGET);
	list.get_base().transition(from, ResourceState::PIXEL_SHADER_RESOURCE);

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
	list.get_base().transition(to, ResourceState::RENDER_TARGET);
	list.get_base().transition(from.resource, ResourceState::PIXEL_SHADER_RESOURCE);

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