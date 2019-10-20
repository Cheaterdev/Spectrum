#include "pch.h"





template <class T>
struct MipMapSignature : public T
{
	using T::T;

	typename T::template ConstBuffer	<0, Render::ShaderVisibility::ALL, 0>									constants = this;
	typename T::template Table			<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 4>	source = this;
	typename T::template Table			<2, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 4>	target = this;


	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearClampDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };

	typename T::template Sampler<2, Render::ShaderVisibility::ALL, 0> point_border{ Render::Samplers::SamplerPointBorderDesc, this };
};





MipMapGenerator::MipMapGenerator()
{
	std::lock_guard<std::mutex> g(m);


	auto root_sig = MipMapSignature<SignatureCreator>().create_root();
	Render::ComputePipelineStateDesc dcdesc;
	Render::RootSignatureDesc root_desc;

	dcdesc.root_signature = root_sig;
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


		{
			Render::PipelineStateDesc state_desc;
			state_desc.root_signature = root_sig;

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



	{
		Render::PipelineStateDesc state_desc;
		state_desc.root_signature = root_sig;
		/*
		{
			Render::RootSignatureDesc root_desc;
		
		//	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1);
			root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
			state_desc.root_signature.reset(new Render::RootSignature(root_desc));
		}*/
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
	//auto view = tex->texture_2d();
	auto timer = compute_context.get_base().start(L"downsampling");

	MipMapSignature<Signature> shader_data(&compute_context);


	//list->transition(tex.get(), before, after);
	//  return;//
	//    Render::ComputePipelineState::ptr& compute = linear[0];
	//   tex->change_state(list, before, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE, 0);
	compute_context.get_base().transition(tex, Render::ResourceState::UNORDERED_ACCESS);
	compute_context.set_signature(linear[0]->desc.root_signature);
	shader_data.source[0] = view->get_srv();
	// compute_context.set(3, samplers);
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
		shader_data.constants.set_raw(cb);
		for (uint32_t i = 0; i < NumMips; i++)
			shader_data.target[i]=view->get_uav(TopMip + 1 + i);
		compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));
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

	MipMapSignature<Signature> shader_data(&compute_context);

	shader_data.source[0]=  tex->texture_2d()->get_static_srv();
	shader_data.target[0] =  to->texture_2d()->get_static_uav();
	compute_context.dispach(ivec2(tex->get_desc().Width, tex->get_desc().Height), ivec2(8, 8));
}


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


void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr& to, Render::Texture::ptr& from)
{
	list.get_base().transition(to, ResourceState::RENDER_TARGET);
	list.get_base().transition(from, ResourceState::PIXEL_SHADER_RESOURCE);

	list.set_pipeline(copy_texture_state);
	MipMapSignature<Signature> shader_data(&list);

	auto& view = to->texture_2d();

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());


	shader_data.source[0] = from->texture_2d()->get_srv();
	list.set_rtv(1, view->get_rtv(), Handle());
	list.draw(4);
	list.get_base().transition(from, ResourceState::RENDER_TARGET);

}