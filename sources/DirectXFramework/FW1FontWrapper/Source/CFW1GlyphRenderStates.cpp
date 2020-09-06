// CFW1GlyphRenderStates.cpp

#include "pch.h"

#include "CFW1GlyphRenderStates.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper
{



    // Construct
    CFW1GlyphRenderStates::CFW1GlyphRenderStates() :
        m_featureLevel(D3D_FEATURE_LEVEL_9_1),

        m_hasGeometryShader(false)

    {
    }


    // Destruct
    CFW1GlyphRenderStates::~CFW1GlyphRenderStates()
    {
    }


    // Init
    HRESULT CFW1GlyphRenderStates::initRenderResources(
        IFW1Factory* pFW1Factory,
        bool wantGeometryShader,
        bool anisotropicFiltering
    )
    {
        HRESULT hResult = initBaseObject(pFW1Factory);

        if (FAILED(hResult))
            return hResult;

        m_featureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;
	
		Render::PipelineStateDesc desc;
        desc.root_signature = get_Signature(Layouts::DefaultLayout);

        desc.vertex = Render::vertex_shader::get_resource({ "shaders/font/vsSimple.hlsl", "VS", 0, {} });
        desc.pixel = Render::pixel_shader::get_resource({ "shaders/font/psSimple.hlsl", "PS", 0, {} });
        desc.layout.inputs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        desc.layout.inputs.push_back({ "GLYPHCOLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		desc.blend.render_target[0].enabled = false;
        ///////////////////////////////////////////////////
        desc.layout.inputs.clear();
        desc.layout.inputs.push_back({ "POSITIONINDEX", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        desc.layout.inputs.push_back({ "GLYPHCOLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        ///////////////////////////////////////////////////
        desc.vertex = Render::vertex_shader::get_resource({ "shaders/font/vsEmpty.hlsl", "VS", 0, {} });
        desc.geometry = Render::geometry_shader::get_resource({ "shaders/font/gsSimple.hlsl", "GS", 0, {} });
        desc.pixel = Render::pixel_shader::get_resource({ "shaders/font/psSimple.hlsl", "PS", 0, {} });
        geometry_state =  Render::PipelineState::create(desc,"geometry_state");
        ///////////////////////////////////////////////////
        desc.geometry = Render::geometry_shader::get_resource({ "shaders/font/gsClip.hlsl", "GS", 0, {} });
        geometry_state_clip = Render::PipelineState::create(desc, "geometry_state_clip");

        // Create all needed resources
        if (SUCCEEDED(hResult))
            hResult = createQuadShaders();

        if (SUCCEEDED(hResult))
            hResult = createPixelShaders();

        if (SUCCEEDED(hResult))
            hResult = createConstantBuffer();

        if (SUCCEEDED(hResult))
            hResult = createRenderStates(anisotropicFiltering);

        if (SUCCEEDED(hResult) && wantGeometryShader)
        {
            hResult = createGlyphShaders();

            if (FAILED(hResult))
                hResult = S_OK;
        }

        if (SUCCEEDED(hResult))
            hResult = S_OK;

        return hResult;
    }


    // Create quad shaders
    HRESULT CFW1GlyphRenderStates::createQuadShaders()
    {
        /*	vertex_state->v_shader = DX11::vertex_shader::get_resource({ "shaders/font/vsSimple.hlsl", "VS", 0, {} });
        	vertex_state_clip->v_shader = DX11::vertex_shader::get_resource({ "shaders/font/vsClip.hlsl", "VS", 0, {} });
        	DX11::input_layout_header layout;
        	layout.inputs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        	layout.inputs.push_back({ "GLYPHCOLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
        	vertex_state->layout = DX11::input_layout::get_resource(layout, vertex_state->v_shader.get());
        	vertex_state_clip->layout = DX11::input_layout::get_resource(layout, vertex_state_clip->v_shader.get());*/
        return S_OK;
    }

    // Create point to quad geometry shader
    HRESULT CFW1GlyphRenderStates::createGlyphShaders()
    {
        if (m_featureLevel < D3D_FEATURE_LEVEL_10_0)
            return E_FAIL;

        /*	geometry_state->v_shader = DX11::vertex_shader::get_resource({ "shaders/font/vsEmpty.hlsl", "VS", 0, {} });
        	geometry_state_clip->v_shader = DX11::vertex_shader::get_resource({ "shaders/font/vsEmpty.hlsl", "VS", 0, {} });

        	geometry_state->g_shader = DX11::geometry_shader::get_resource({ "shaders/font/gsSimple.hlsl", "GS", 0, {} });
        	geometry_state_clip->g_shader = DX11::geometry_shader::get_resource({ "shaders/font/gsClip.hlsl", "GS", 0, {} });
        	DX11::input_layout_header layout;
        	layout.inputs.push_back({ "POSITIONINDEX", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        	layout.inputs.push_back({ "GLYPHCOLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

        	geometry_state->layout = DX11::input_layout::get_resource(layout, geometry_state->v_shader.get());
        	geometry_state_clip->layout = DX11::input_layout::get_resource(layout, geometry_state_clip->v_shader.get());
        	*/
        m_hasGeometryShader = true;
        return S_OK;
    }

    // Create pixel shaders
    HRESULT CFW1GlyphRenderStates::createPixelShaders()
    {
        /*	geometry_state->p_shader = DX11::pixel_shader::get_resource({ "shaders/font/psSimple.hlsl", "PS", 0, {} });
        	geometry_state_clip->p_shader = DX11::pixel_shader::get_resource({ "shaders/font/psSimple.hlsl", "PS", 0, {} });
        	vertex_state->p_shader = DX11::pixel_shader::get_resource({ "shaders/font/psSimple.hlsl", "PS", 0, {} });
        	vertex_state_clip->p_shader = DX11::pixel_shader::get_resource({ "shaders/font/psClip.hlsl", "PS", 0, {} });
        	*/
        return S_OK;
    }


    // Create constant buffer
    HRESULT CFW1GlyphRenderStates::createConstantBuffer()
    {
        ////   m_pConstantBuffer.reset(new Render::Buffer<ShaderConstants>(Render::HeapType::DEFAULT));
        //    const_buffer_table = Render::DescriptorHeapManager::get().get_csu()->create_table(1);
        //   m_pConstantBuffer->place_const_buffer(const_buffer_table[0]);
        return S_OK;
    }


    // Create render states
    HRESULT CFW1GlyphRenderStates::createRenderStates(bool anisotropicFiltering)
    {
        // Create blend-state
        /*     D3D11_BLEND_DESC blendDesc;
             ZeroMemory(&blendDesc, sizeof(blendDesc));

             for (int i = 0; i < 4; ++i)
             {
                 blendDesc.RenderTarget[i].BlendEnable = TRUE;
                 blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
                 blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                 blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
                 blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
                 blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
                 blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                 blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
             }

             geometry_state->blend = DX11::blend_state::get_resource(blendDesc);
             geometry_state_clip->blend = DX11::blend_state::get_resource(blendDesc);
             vertex_state->blend = DX11::blend_state::get_resource(blendDesc);
             vertex_state_clip->blend = DX11::blend_state::get_resource(blendDesc);
             // Create sampler state
             D3D11_SAMPLER_DESC samplerDesc;
             ZeroMemory(&samplerDesc, sizeof(samplerDesc));

             if (anisotropicFiltering)
             {
                 samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                 samplerDesc.MaxAnisotropy = 2;

                 if (m_featureLevel >= D3D_FEATURE_LEVEL_9_2)
                     samplerDesc.MaxAnisotropy = 5;
             }

             else
                 samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

             samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
             samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
             samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
             samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
             m_pSamplerState = DX11::sampler_state::get_resource(samplerDesc);
             // Create rasterizer state
             D3D11_RASTERIZER_DESC rasterizerDesc;
             ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
             rasterizerDesc.FillMode = D3D11_FILL_SOLID;
             rasterizerDesc.CullMode = D3D11_CULL_NONE;
             rasterizerDesc.FrontCounterClockwise = FALSE;
             rasterizerDesc.DepthClipEnable = TRUE;
             geometry_state->rasterizer = DX11::rasterizer_state::get_resource(rasterizerDesc);
             geometry_state_clip->rasterizer =  DX11::rasterizer_state::get_resource(rasterizerDesc);
             vertex_state->rasterizer =  DX11::rasterizer_state::get_resource(rasterizerDesc);
             vertex_state_clip->rasterizer =  DX11::rasterizer_state::get_resource(rasterizerDesc);
             // Create depth-stencil state
             D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
             ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
             depthStencilDesc.DepthEnable = FALSE;
             geometry_state->depth_stencil = DX11::depth_stencil_state::get_resource(depthStencilDesc);
             geometry_state_clip->depth_stencil = DX11::depth_stencil_state::get_resource(depthStencilDesc);
             vertex_state->depth_stencil = DX11::depth_stencil_state::get_resource(depthStencilDesc);
             vertex_state_clip->depth_stencil = DX11::depth_stencil_state::get_resource(depthStencilDesc);*/
     //   pixel_sampler_table = Render::DescriptorHeapManager::get().gpu_smp->create_table(1);
        D3D12_SAMPLER_DESC samplerDesc = {};

        if (anisotropicFiltering)
        {
            samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
            samplerDesc.MaxAnisotropy = 2;

            if (m_featureLevel >= D3D_FEATURE_LEVEL_9_2)
                samplerDesc.MaxAnisotropy = 5;
        }

        else
            samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
      //  Render::Device::get().create_sampler(samplerDesc, pixel_sampler_table[0].cpu);
        return S_OK;
    }
}// namespace FW1FontWrapper
