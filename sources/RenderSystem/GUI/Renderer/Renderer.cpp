#include "pch.h"
#include "Renderer.h"

#include <slots.h>


namespace GUI
{


    void Renderer::draw_area(base::ptr obj, DX12::context& c)
    {
        draw(c, area_tex, obj->get_render_bounds());
    }

    void Renderer::draw_container(base::ptr obj, DX12::context& c)
    {
        draw(c, container_tex, obj->get_render_bounds());
    }


    void Renderer::draw_virtual(base::ptr obj, DX12::context& c)
    {
        draw(c, virtual_tex, obj->get_render_bounds());
    }


    void Renderer::draw_color(DX12::context& c, float4 color, rect r)
    {
		flush(c);
       // c.command_list->get_graphics().set_signature(root_signature);
		//UISignature<Signature> sig(&c.command_list->get_graphics());
		//sig.pixel_samples = sampler_table;
      //  c.command_list->get_graphics().set(3, sampler_table);
        simple_rect->draw(c, color, r);
    }

    void Renderer::set(DX12::context& c)
    {
	//	UISignature<Signature> sig(&c.command_list->get_graphics());
	//	sig.pixel_samples = sampler_table;
		//c.command_list->get_graphics().set(3, sampler_table);
    }
	void Renderer::flush(DX12::context& c)
	{
		nine_patch->flush(c);
	}
    Renderer::Renderer()
    {
      /*  DX12::RootSignatureDesc root_desc;
        root_desc[0] = DX12::DescriptorConstBuffer(0, DX12::ShaderVisibility::VERTEX);
        root_desc[1] = DX12::DescriptorConstBuffer(0, DX12::ShaderVisibility::PIXEL);
        root_desc[2] = DX12::DescriptorTable(DX12::DescriptorRange::SRV, DX12::ShaderVisibility::PIXEL, 0, 5);
        root_desc[3] = DX12::DescriptorTable(DX12::DescriptorRange::SAMPLER, DX12::ShaderVisibility::PIXEL, 0, 3);
        root_desc[4] = DX12::DescriptorConstBuffer(0, DX12::ShaderVisibility::GEOMETRY);
		root_desc[5] = DX12::DescriptorSRV(0, DX12::ShaderVisibility::VERTEX,1);
		root_desc[6] = DX12::DescriptorSRV(1, DX12::ShaderVisibility::VERTEX, 1);
		root_desc[7] = DX12::DescriptorTable(DX12::DescriptorRange::SRV, DX12::ShaderVisibility::PIXEL, 0, 1,1);
		
        root_signature.reset(new DX12::RootSignature(root_desc));/*
		{
			{ DX12::DescriptorRange::CBV, DX12::ShaderVisibility::VERTEX, 0, 2 },
			{ DX12::DescriptorRange::CBV, DX12::ShaderVisibility::PIXEL, 0, 5 },
			{ DX12::DescriptorRange::SRV, DX12::ShaderVisibility::PIXEL, 0, 5 },

			{ DX12::DescriptorRange::SAMPLER, DX12::ShaderVisibility::PIXEL, 0, 3 },
			{ DX12::DescriptorRange::CBV, DX12::ShaderVisibility::GEOMETRY, 0, 5 }

		}));*/


	//	root_signature = UISignature<SignatureCreator>().create_root();

	//	root_signature->set_unfixed(6);
        /*sampler_table = DX12::DescriptorHeapManager::get().get_samplers()->create_table(3);
        //   descriptor_sr_cb.reset(new DX12::DescriptorHeap(512, DX12::DescriptorHeapType::CBV_SRV_UAV, DX12::DescriptorHeapFlags::SHADER_VISIBLE));
        //   descriptor_sampler.reset(new DX12::DescriptorHeap(2, DX12::DescriptorHeapType::SAMPLER, DX12::DescriptorHeapFlags::SHADER_VISIBLE));
        D3D12_SAMPLER_DESC wrapSamplerDesc = {};
        wrapSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        wrapSamplerDesc.MinLOD = 0;
        wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        wrapSamplerDesc.MipLODBias = 0.0f;
        wrapSamplerDesc.MaxAnisotropy = 8;
        wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
        DX12::Device::get().create_sampler(wrapSamplerDesc, sampler_table[0].cpu);
        wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        DX12::Device::get().create_sampler(wrapSamplerDesc, sampler_table[1].cpu);
        wrapSamplerDesc.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        wrapSamplerDesc.MaxLOD = 1;
        DX12::Device::get().create_sampler(wrapSamplerDesc, sampler_table[2].cpu);
        */
        nine_patch.reset(new NinePatch());
        simple_rect.reset(new SimpleRect());
        area_tex.texture = DX12::Texture::get_resource({ "textures/gui/edit.png", false, false });
        virtual_tex.texture = DX12::Texture::get_resource({ "textures/gui/virtual.png", false , false });
        container_tex.texture = DX12::Texture::get_resource({ "textures/gui/background.png", false, false });
        area_tex.padding = { 5, 5, 5, 5 };
        virtual_tex.padding = { 5, 5, 5, 5 };
		container_tex.padding = {2,2,2,2};
    }

    void Renderer::draw(DX12::context& c, DX12::PipelineState::ptr state, rect r)
    {
        nine_patch->draw(c, state, r);
    }

    void Renderer::draw(DX12::context& c, GUI::Texture& item, rect r)
    {
        nine_patch->draw(c, item, r);
    }

    SimpleRect::SimpleRect()
    {
    }

    void SimpleRect::draw(DX12::context& c, float4 color, rect r)
	{
		Slots::ColorRect color_data;


        auto vertexes = (vec2*)color_data.GetPos();

        vertexes[0] = float2(0, 0);
        vertexes[1] = float2(0, r.size.y);
        vertexes[2] = float2(r.size.x, 0);
        vertexes[3] = float2(r.size.x, r.size.y);

		auto &clip = c.ui_clipping;


		for (int i = 0; i < 4; i++)
		{
			vertexes[i] += float2(r.pos) + c.offset;
			vertexes[i] = float2::max(vertexes[i], float2(clip.left_top));
			vertexes[i] = float2::min(vertexes[i], float2(clip.right_bottom));

		}
           
        for (int i = 0; i < 4; i++)
        {
            float2 t = 2 * vertexes[i] / c.window_size - float2(1, 1);
            vertexes[i] = { t.x, -t.y };
        }

      
        color_data.GetColor() = color;

        color_data.set(c.command_list->get_graphics());

        c.command_list->get_graphics().set_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        c.command_list->get_graphics().set_pipeline(GetPSO<PSOS::SimpleRect>());
        c.command_list->get_graphics().draw(4);

    }

}
