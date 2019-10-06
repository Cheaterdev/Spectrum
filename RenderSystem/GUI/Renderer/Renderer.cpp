#include "pch.h"



namespace GUI
{


    void Renderer::draw_area(base::ptr obj, Render::context& c)
    {
        draw(c, area_tex, obj->get_render_bounds());
    }

    void Renderer::draw_container(base::ptr obj, Render::context& c)
    {
        draw(c, container_tex, obj->get_render_bounds());
    }


    void Renderer::draw_virtual(base::ptr obj, Render::context& c)
    {
        draw(c, virtual_tex, obj->get_render_bounds());
    }


    void Renderer::draw_color(Render::context& c, float4 color, rect r)
    {
		flush(c);
        c.command_list->get_graphics().set_signature(root_signature);
		UISignature<Signature> sig(&c.command_list->get_graphics());
		sig.pixel_samples = sampler_table;
      //  c.command_list->get_graphics().set(3, sampler_table);
        simple_rect->draw(c, color, r);
    }

    void Renderer::set(Render::context& c)
    {
		UISignature<Signature> sig(&c.command_list->get_graphics());
		sig.pixel_samples = sampler_table;
		//c.command_list->get_graphics().set(3, sampler_table);
    }
	void Renderer::flush(Render::context& c)
	{
		nine_patch->flush(c);
	}
    Renderer::Renderer()
    {
      /*  Render::RootSignatureDesc root_desc;
        root_desc[0] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::VERTEX);
        root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::PIXEL);
        root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 5);
        root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::PIXEL, 0, 3);
        root_desc[4] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::GEOMETRY);
		root_desc[5] = Render::DescriptorSRV(0, Render::ShaderVisibility::VERTEX,1);
		root_desc[6] = Render::DescriptorSRV(1, Render::ShaderVisibility::VERTEX, 1);
		root_desc[7] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1,1);
		
        root_signature.reset(new Render::RootSignature(root_desc));/*
		{
			{ Render::DescriptorRange::CBV, Render::ShaderVisibility::VERTEX, 0, 2 },
			{ Render::DescriptorRange::CBV, Render::ShaderVisibility::PIXEL, 0, 5 },
			{ Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 5 },

			{ Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::PIXEL, 0, 3 },
			{ Render::DescriptorRange::CBV, Render::ShaderVisibility::GEOMETRY, 0, 5 }

		}));*/


		root_signature = UISignature<SignatureCreator>().create_root();

		root_signature->set_unfixed(7);
        sampler_table = Render::DescriptorHeapManager::get().get_samplers()->create_table(3);
        //   descriptor_sr_cb.reset(new Render::DescriptorHeap(512, Render::DescriptorHeapType::CBV_SRV_UAV, Render::DescriptorHeapFlags::SHADER_VISIBLE));
        //   descriptor_sampler.reset(new Render::DescriptorHeap(2, Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapFlags::SHADER_VISIBLE));
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
        Render::Device::get().create_sampler(wrapSamplerDesc, sampler_table[0].cpu);
        wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        Render::Device::get().create_sampler(wrapSamplerDesc, sampler_table[1].cpu);
        wrapSamplerDesc.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        wrapSamplerDesc.MaxLOD = 1;
        Render::Device::get().create_sampler(wrapSamplerDesc, sampler_table[2].cpu);

        nine_patch.reset(new NinePatch(root_signature));
        simple_rect.reset(new SimpleRect(root_signature));
        area_tex.texture = Render::Texture::get_resource({ "textures/gui/edit.png", false, false });
        virtual_tex.texture = Render::Texture::get_resource({ "textures/gui/virtual.png", false , false });
        container_tex.texture = Render::Texture::get_resource({ "textures/gui/background.png", false, false });
        area_tex.padding = { 5, 5, 5, 5 };
        virtual_tex.padding = { 5, 5, 5, 5 };
		container_tex.padding = {2,2,2,2};
    }

    void Renderer::draw(Render::context& c, Render::PipelineState::ptr& state, rect r)
    {
        nine_patch->draw(c, state, r);
    }

    void Renderer::draw(Render::context& c, GUI::Texture& item, rect r)
    {
        nine_patch->draw(c, item, r);
    }

    SimpleRect::SimpleRect(Render::RootSignature::ptr root)
    {
        Render::PipelineStateDesc state_desc;
        state_desc.root_signature = root;
        state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\gui\\rect.hlsl", "PS_COLOR", 0, {} });
        state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\gui\\rect.hlsl", "VS", 0, {} });
        state_desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        state_desc.layout.inputs.push_back({ "SV_POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        state.reset(new Render::PipelineState(state_desc));
        vertexes.resize(6);
        vblist.resize(1);
    }

    void SimpleRect::draw(Render::context& c, float4 color, rect r)
    {
        Render::PipelineState::ptr& pipeline_state = state;
        vertexes[5].pos = float2(0, 0);
        vertexes[4].pos = float2(0, r.size.y);
        vertexes[3].pos = float2(r.size.x, r.size.y);
        vertexes[2].pos = float2(0, 0);
        vertexes[1].pos = float2(r.size.x, r.size.y);
        vertexes[0].pos = float2(r.size.x, 0);

		auto &clip = c.ui_clipping;


		for (int i = 0; i < 6; i++)
		{
			vertexes[i].pos += float2(r.pos) + c.offset;
			vertexes[i].pos = float2::max(vertexes[i].pos, float2(clip.left_top));
			vertexes[i].pos = float2::min(vertexes[i].pos, float2(clip.right_bottom));

		}
           
        for (int i = 0; i < 6; i++)
        {
            float2 t = 2 * vertexes[i].pos / c.window_size - float2(1, 1);
            vertexes[i].pos = { t.x, -t.y };
        }

        //    vertexes.update(c.command_list);
        vblist[0] = c.command_list->get_graphics().place_vertex_buffer(vertexes);
		UISignature<Signature> sig(&c.command_list->get_graphics());


        c.command_list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        c.command_list->get_graphics().set_vertex_buffers(0, vblist);
		sig.pixel_const_buffer.set_raw(color);
        c.command_list->get_graphics().set_pipeline(pipeline_state);
        c.command_list->get_graphics().draw(6);
    }

}
