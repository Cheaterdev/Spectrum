
#include "pch.h"
#include "PipelineState.h"

namespace DX12
{


#define LESS_ID(x) if((l.x ? l.x->id : -5)<(r.x ? r.x->id : -5)) return true; if((r.x ? r.x->id : -5)<(l.x ? l.x->id : -5)) return false;

#define OP_SHADER(x,y)\
if (l.x == r.x)\
{\
	y\
}\
else\
{\
	if(l.x&&r.x)\
		return l.x->id < r.x->id;\
	else\
	return l.x < r.x;\
}


#define OP_E_SHADER(x)\
if((l.x?l.x->id:-5)!=(r.x?r.x->id:-5)) return false;


    bool operator<(const input_layout_header& l, const input_layout_header& r)
    {
        OP(inputs.size(),

           for (unsigned int i = 0; i < l.inputs.size(); i++)
    {
        OP(inputs[i].SemanticIndex,
           OP(inputs[i].Format,
              OP(inputs[i].InputSlot,
                 OP(inputs[i].AlignedByteOffset,
                    OP(inputs[i].InputSlotClass,
                       OP(inputs[i].InstanceDataStepRate,
                          return strcmp(l.inputs[i].SemanticName, r.inputs[i].SemanticName) < (int)0;
                         )
                      )
                   )
                )
             )
          );
        }
          );
        return false;
    }
    bool operator==(const input_layout_header& l, const input_layout_header& r)
    {
        OP_E(inputs.size())

        for (unsigned int i = 0; i < l.inputs.size(); i++)
        {
            OP_E(inputs[i].SemanticIndex)
            OP_E(inputs[i].Format)
            OP_E(inputs[i].InputSlot)
            OP_E(inputs[i].AlignedByteOffset)
            OP_E(inputs[i].InputSlotClass)
            OP_E(inputs[i].InstanceDataStepRate)

            if (strcmp(l.inputs[i].SemanticName, r.inputs[i].SemanticName))
                return false;
        }

        return true;
    }
    bool operator==(const RasterizerState& l, const RasterizerState& r)
    {
        OP_E(cull_mode);
        OP_E(fill_mode);
        OP_E(conservative);
        return true;
    }
    bool operator==(const RTVState& l, const RTVState& r)
    {
        OP_E(enable_depth)
        OP_E(enable_depth_write)
        OP_E(ds_format);
        OP_E(rtv_formats.size())
        OP_E(func)
			OP_E(enable_stencil)
			OP_E(stencil_read_mask)
			OP_E(stencil_write_mask)

			OP_E(stencil_desc.StencilFailOp)
			OP_E(stencil_desc.StencilDepthFailOp)
			OP_E(stencil_desc.StencilPassOp)
			OP_E(stencil_desc.StencilFunc)

        return true;
    }
    bool operator==(const BlendState& l, const BlendState& r)
    {
        OP_E(independ_blend)

        for (int i = 0; i < 8; i++)
            OP_E(render_target[i])
            return true;
    }

    bool operator==(const RenderTarget& l, const RenderTarget& r)
    {
        OP_E(enabled)
        OP_E(source)
			OP_E(dest) 
			OP_E(source_alpha)
			OP_E(dest_alpha)
        return true;
    }
    bool operator==(const PipelineStateDesc& l, const PipelineStateDesc& r)
    {
        OP_E(root_signature)
        OP_E(blend)
        OP_E(rasterizer)
        OP_E(rtv)
        OP_E_SHADER(vertex)
        OP_E_SHADER(pixel)
        OP_E_SHADER(geometry)
        OP_E_SHADER(hull)
        OP_E_SHADER(domain)
        OP(topology)
        OP(hull)
        OP_E(layout)
        return true;
    }
    bool operator<(const RasterizerState& l, const RasterizerState& r)
    {
        LESS(cull_mode);
        LESS(fill_mode);
        LESS(conservative);
        return false;
    }
	bool operator<(const RTVState& l, const RTVState& r)
	{
		LESS(enable_depth)
			LESS(enable_depth_write)
			LESS(rtv_formats.size())
			LESS(ds_format)
			LESS(func)

			LESS(enable_stencil)
			LESS(stencil_read_mask)
			LESS(stencil_write_mask)

			LESS(stencil_desc.StencilFailOp)
			LESS(stencil_desc.StencilDepthFailOp)
			LESS(stencil_desc.StencilPassOp)
			LESS(stencil_desc.StencilFunc)



			return false;
	}
	bool operator<(const BlendState& l, const BlendState& r)
    {
        LESS(independ_blend)

        for (int i = 0; i < 8; i++)
            LESS(render_target[i])
            return false;
    }


    bool operator<(const RenderTarget& l, const RenderTarget& r)
    {
        LESS(enabled)
        LESS(source)
        LESS(dest)
        return false;
    }

    bool operator<(const PipelineStateDesc& l, const PipelineStateDesc& r)
    {
        LESS(root_signature)
        LESS(blend)
        LESS(rasterizer)
        LESS(rtv)
        LESS(topology)
        LESS(layout)
        LESS_ID(vertex)
        LESS_ID(pixel)
        LESS_ID(geometry)
        LESS_ID(hull)
        LESS_ID(domain)
        /*
        OP(root_signature,
           OP(blend,
              OP(rasterizer,
                 OP(rtv,
                    OP_SHADER(vertex,
                              OP_SHADER(pixel,
                                        OP_SHADER(geometry,
                                                OP_SHADER(hull,
                                                        OP_SHADER(domain,
                                                                OP(topology,
                                                                        OP_LAST(layout)
                                                                  )
                                                                 )
                                                         )
                                                 )
                                       )
                             )
                   )
                )
             )
          )*/
        return false;
    }
	 void PipelineState::on_change()
	{
		auto t = CounterManager::get().start_count<PipelineState>();
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		//                psoDesc.InputLayout = inputLayoutDesc;
		D3D12_INPUT_LAYOUT_DESC layout_desc;
		layout_desc.NumElements = static_cast<UINT>(desc.layout.inputs.size());
		layout_desc.pInputElementDescs = desc.layout.inputs.data();
		psoDesc.InputLayout = layout_desc;

		if (desc.root_signature)
			psoDesc.pRootSignature = desc.root_signature->get_native().Get();

		if (desc.vertex)
			psoDesc.VS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.vertex->get_blob().data())), static_cast<UINT>(desc.vertex->get_blob().size()) };

		if (desc.pixel)
			psoDesc.PS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.pixel->get_blob().data())), static_cast<UINT>(desc.pixel->get_blob().size()) };

		if (desc.geometry)
			psoDesc.GS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.geometry->get_blob().data())), static_cast<UINT>(desc.geometry->get_blob().size()) };

		if (desc.domain)
			psoDesc.DS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.domain->get_blob().data())), static_cast<UINT>(desc.domain->get_blob().size()) };

		if (desc.hull)
			psoDesc.HS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.hull->get_blob().data())), static_cast<UINT>(desc.hull->get_blob().size()) };

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

		psoDesc.RasterizerState.CullMode = desc.rasterizer.cull_mode;

		psoDesc.RasterizerState.FillMode = desc.rasterizer.fill_mode;

		psoDesc.RasterizerState.ConservativeRaster = desc.rasterizer.conservative ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		psoDesc.BlendState.IndependentBlendEnable = desc.blend.independ_blend;

		for (unsigned int i = 0; i < 8; i++)
		{
			psoDesc.BlendState.RenderTarget[i].BlendEnable = desc.blend.render_target[i].enabled;
			psoDesc.BlendState.RenderTarget[i].SrcBlend = desc.blend.render_target[i].source;
			psoDesc.BlendState.RenderTarget[i].DestBlend = desc.blend.render_target[i].dest;
			psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = desc.blend.render_target[i].dest_alpha;
			psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = desc.blend.render_target[i].source_alpha;

		}

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);;
		psoDesc.DepthStencilState.DepthWriteMask = desc.rtv.enable_depth_write ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.DepthEnable = desc.rtv.enable_depth;
		psoDesc.DepthStencilState.DepthFunc = desc.rtv.func;

		psoDesc.DepthStencilState.StencilEnable = desc.rtv.enable_stencil;
		psoDesc.DepthStencilState.StencilReadMask = desc.rtv.stencil_read_mask;
		psoDesc.DepthStencilState.StencilWriteMask = desc.rtv.stencil_write_mask;
		psoDesc.DepthStencilState.FrontFace = desc.rtv.stencil_desc;
		psoDesc.DepthStencilState.BackFace = desc.rtv.stencil_desc;



		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = desc.topology;
		psoDesc.NumRenderTargets = static_cast<UINT>(desc.rtv.rtv_formats.size());

		for (unsigned int i = 0; i < desc.rtv.rtv_formats.size(); i++)
			psoDesc.RTVFormats[i] = desc.rtv.rtv_formats[i];

		psoDesc.DSVFormat = desc.rtv.ds_format;
		psoDesc.SampleDesc.Count = 1;
		if(m_pipelineState)
		Device::get().unused(m_pipelineState);

 		TEST(Device::get().get_native_device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}
	 ComPtr<ID3D12PipelineState> PipelineStateBase::get_native()
	{
		return m_pipelineState;
	}
	 PipelineState::PipelineState(PipelineStateDesc _desc) : desc(_desc)
	{
		on_change();
		register_shader(desc.pixel);
		register_shader(desc.vertex);
		register_shader(desc.hull);
		register_shader(desc.domain);
		register_shader(desc.geometry);
	}
	 PipelineState::~PipelineState()
	{
		unregister_shader(desc.pixel);
		unregister_shader(desc.vertex);
		unregister_shader(desc.hull);
		unregister_shader(desc.domain);
		unregister_shader(desc.geometry);

		if (m_pipelineState)
			Device::get().unused(m_pipelineState);
	}
	 PipelineState::ptr PipelineStateCache::get_cache(PipelineStateDesc & desc)
	{
		return Singleton<PipelineStateCache>::get().cache[desc];
	}
	 void ComputePipelineState::on_change()
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

		if (desc.root_signature)
			psoDesc.pRootSignature = desc.root_signature->get_native().Get();

		if (desc.shader)
			psoDesc.CS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.shader->get_blob().data())), static_cast<UINT>(desc.shader->get_blob().size()) };

		TEST(Device::get().get_native_device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	 ComputePipelineState::~ComputePipelineState()
	{
		unregister_shader(desc.shader);

		if (m_pipelineState)
			Device::get().unused(m_pipelineState);
	}



	 std::shared_ptr<ComputePipelineState> ComputePipelineStateDesc::create()
	 {
		 return std::make_shared<Render::ComputePipelineState>(*this);
	 }
}