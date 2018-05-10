#include "pch.h"


MipMapGenerator::MipMapGenerator()
{
    std::lock_guard<std::mutex> g(m);
    Render::ComputePipelineStateDesc dcdesc;
    Render::RootSignatureDesc root_desc;
    root_desc[0] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
    root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1);
    root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 4);
    root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 1);
    dcdesc.root_signature.reset(new Render::RootSignature(root_desc));
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

    samplers = Render::DescriptorHeapManager::get().get_samplers()->create_table(1);
    D3D12_SAMPLER_DESC wrapSamplerDesc = {};
    wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    wrapSamplerDesc.MinLOD = 0;
    wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    wrapSamplerDesc.MipLODBias = 0.0f;
    wrapSamplerDesc.MaxAnisotropy = 16;
    wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
    Render::Device::get().create_sampler(wrapSamplerDesc, samplers[0].cpu);
    {
        Render::RootSignatureDesc root_desc;
        root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1);
        root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
        root_desc.set_sampler(0, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerPointBorderDesc);
        dcdesc.root_signature.reset(new Render::RootSignature(root_desc));
        dcdesc.shader = Render::compute_shader::get_resource({ "shaders\\downsample_depth.hlsl", "CS", 0 });
        state_downsample_depth.reset(new Render::ComputePipelineState(dcdesc));
    }


	{


		{
			Render::PipelineStateDesc state_desc;
			{
				Render::RootSignatureDesc root_desc;
				root_desc[0] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
				root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
				root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
				root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
				state_desc.root_signature.reset(new Render::RootSignature(root_desc));
			}
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
		{
			Render::RootSignatureDesc root_desc;
			root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1);
			root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
			state_desc.root_signature.reset(new Render::RootSignature(root_desc));
		}
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
    std::lock_guard<std::mutex> g(m);
    auto timer = compute_context.start(L"downsampling");
    //list->transition(tex.get(), before, after);
    //  return;//
    //    Render::ComputePipelineState::ptr& compute = linear[0];
    //   tex->change_state(list, before, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE, 0);
    compute_context.transition(tex, Render::ResourceState::UNORDERED_ACCESS);
    compute_context.set_signature(linear[0]->desc.root_signature);
    compute_context.set_dynamic(1,0, tex->texture_2d()->get_srv());
    compute_context.set(3, samplers);
    uint32_t maps = tex->get_desc().MipLevels - 1;

    for (uint32_t TopMip = 0; TopMip < maps;)
    {
        uint32_t SrcWidth = tex->get_desc().Width >> TopMip;
        uint32_t SrcHeight = tex->get_desc().Height >> TopMip;
        uint32_t DstWidth = SrcWidth >> 1;
        uint32_t DstHeight = SrcHeight >> 1;
        uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;

        if (DirectX::IsSRGB(tex->get_desc().Format))
            compute_context.set_pipeline(gamma[NonPowerOfTwo]);
        else
            compute_context.set_pipeline(linear[NonPowerOfTwo]);

        uint32_t AdditionalMips;
        _BitScanForward((unsigned long*)&AdditionalMips, DstWidth | DstHeight);
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
        compute_context.set_const_buffer(0, cb);
        compute_context.set_dynamic(2,0, tex->texture_2d()->get_uav(TopMip + 1));
        compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));
        compute_context.transition_uav(tex.get());
        TopMip += NumMips;
    }

 //   compute_context.transition(tex, after);
}

void MipMapGenerator::downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr& tex, Render::Texture::ptr& to)
{
    std::lock_guard<std::mutex> g(m);
    compute_context.transition(tex, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
    compute_context.transition(to, Render::ResourceState::UNORDERED_ACCESS);
   compute_context.set_pipeline(state_downsample_depth);
    compute_context.set_dynamic(0,0, tex->texture_2d()->get_static_srv());
    compute_context.set_dynamic(1,0, to->texture_2d()->get_static_uav());
   compute_context.dispach(ivec2(tex->get_desc().Width, tex->get_desc().Height), ivec2(8, 8));
}


void MipMapGenerator::generate_quality(Render::GraphicsContext& list,camera* cam, G_Buffer& buffer)
{
	std::lock_guard<std::mutex> g(m);
	{
		list.set_pipeline(quality_buffer_state);
		auto & view = buffer.quality_color->texture_2d();

		list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		list.set_viewport(view->get_viewport());
		list.set_scissor(view->get_scissor());
		list.set(0, cam->get_const_buffer());
		list.set_dynamic(1, 0, buffer.normal_tex->texture_2d()->get_srv());
		list.set_dynamic(1, 1, buffer.depth_tex_mips->texture_2d()->get_srv());
		list.set_dynamic(1, 2, buffer.normal_tex->texture_2d()->get_srv());
		list.set_dynamic(1, 3, buffer.depth_tex->texture_2d()->get_srv());
		buffer.quality_color_table.set(list);
		list.draw(4);

	}

	
	{
		buffer.quality_table.clear_stencil(list, 0);
	
		list.set_pipeline(quality_buffer_stencil_state);
		auto & view = buffer.quality_stencil->texture_2d();
		auto & view_color = buffer.quality_color->texture_2d();

		list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		list.set_viewport(view->get_viewport());
		list.set_scissor(view->get_scissor());
		list.set(0, cam->get_const_buffer());

		list.set_dynamic(1, 0, buffer.normal_tex->texture_2d()->get_srv());
		list.set_dynamic(1, 1, buffer.depth_tex_mips->texture_2d()->get_srv());
		list.set_dynamic(1, 2, buffer.quality_color->texture_2d()->get_srv());
		list.set_dynamic(1, 3, buffer.quality_color->texture_2d()->get_srv());


	//	list.set_dynamic(1, 0, view_color->get_srv());
		buffer.quality_table.set(list);


		list.get_native_list()->OMSetStencilRef(1);
		list.draw(4);


		list.set_pipeline(quality_buffer_refl_stencil_state);
		list.get_native_list()->OMSetStencilRef(2);
		list.draw(4);
	}




	/*list.set_pipeline(quality_buffer_state);
	auto & view = buffer.quality_stencil->texture_2d();
		//buffer.quality_table.clear_depth(list);
		buffer.quality_table.clear_stencil(list, 0);

	buffer.quality_table.set(list);
//	list.set_rtv(1, Handle(), view->get_ds());



	*/


	
}


void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr& to, Render::Texture::ptr& from)
{
	list.transition(to, ResourceState::RENDER_TARGET);
	list.transition(from, ResourceState::PIXEL_SHADER_RESOURCE);

	list.set_pipeline(copy_texture_state);
	auto & view = to->texture_2d();

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());


	list.set_dynamic(0, 0, from->texture_2d()->get_srv());
	list.set_rtv(1, view->get_rtv(), Handle());
	list.draw(4);
	list.transition(from, ResourceState::RENDER_TARGET);

}