#include "pch.h"
#include "PipelineState.h"


#define E(x) 	if ( l.x != r.x) return false;
#define C(x) 	if (auto cmp = l.x <=> r.x; cmp != 0) return cmp;

bool operator==(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r)
{
	E(StencilDepthFailOp)
		E(StencilFailOp)
		E(StencilFunc)
		E(StencilPassOp)


		return true;
}


std::strong_ordering operator<=>(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r)
{
	C(StencilDepthFailOp)
		C(StencilFailOp)
		C(StencilFunc)
		C(StencilPassOp)


		return std::strong_ordering::equal;
}


namespace DX12
{

	void PipelineState::on_change()
	{
		root_signature = desc.root_signature;

		auto t = CounterManager::get().start_count<PipelineState>();


		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		std::vector<D3D12_INPUT_ELEMENT_DESC> elements;

		for (const auto& item : desc.layout.inputs)
			elements.push_back(item.create_native());

		psoDesc.InputLayout.NumElements = static_cast<UINT>(elements.size());
		psoDesc.InputLayout.pInputElementDescs = elements.data();

		if (desc.root_signature)
			psoDesc.pRootSignature = desc.root_signature->get_native().Get();

		slots.clear();
		if (desc.vertex)
		{
			psoDesc.VS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.vertex->get_blob().data())), static_cast<UINT>(desc.vertex->get_blob().size()) };
			slots.merge(desc.vertex->slots_usage);
		}
		if (desc.pixel)
		{
			psoDesc.PS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.pixel->get_blob().data())), static_cast<UINT>(desc.pixel->get_blob().size()) };
			slots.merge(desc.pixel->slots_usage);
		}
		if (desc.geometry)
		{
			psoDesc.GS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.geometry->get_blob().data())), static_cast<UINT>(desc.geometry->get_blob().size()) };
			slots.merge(desc.geometry->slots_usage);
		}
		if (desc.domain)
		{
			psoDesc.DS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.domain->get_blob().data())), static_cast<UINT>(desc.domain->get_blob().size()) };
			slots.merge(desc.domain->slots_usage);
		}
		if (desc.hull)
		{
			psoDesc.HS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.hull->get_blob().data())), static_cast<UINT>(desc.hull->get_blob().size()) };
			slots.merge(desc.hull->slots_usage);
		}
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
		psoDesc.SampleDesc.Quality = 0;

		if (!cache.empty())
		{
			psoDesc.CachedPSO.pCachedBlob = cache.c_str();
			psoDesc.CachedPSO.CachedBlobSizeInBytes = cache.size();
		}
		else
		{
			//	assert(false);
		}

		//m_pipelineState = PipelineLibrary::get().create(desc.name, psoDesc);

		HRESULT hr = (Device::get().get_native_device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&tracked_info->m_pipelineState)));

		if (hr != S_OK)
		{
			psoDesc.CachedPSO = {};
			TEST(Device::get().get_native_device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&tracked_info->m_pipelineState)));
		}



		cache.clear();

		debuggable = false;
		debuggable |= desc.vertex && desc.vertex->depends_on("DebugInfo");
		debuggable |= desc.pixel && desc.pixel->depends_on("DebugInfo");
		debuggable |= desc.geometry && desc.geometry->depends_on("DebugInfo");
		debuggable |= desc.domain && desc.domain->depends_on("DebugInfo");
		debuggable |= desc.hull && desc.hull->depends_on("DebugInfo");
	

		name = desc.name;
	}

	PipelineState::ptr PipelineState::create(PipelineStateDesc& desc, std::string name)
	{
		return PipelineStateCache::get_cache(desc, name);
	}
	ComputePipelineState::ptr ComputePipelineState::create(ComputePipelineStateDesc& desc, std::string name)
	{
		return PipelineStateCache::get_cache(desc, name);
	}

	ComPtr<ID3D12PipelineState> PipelineStateBase::get_native()
	{
		return tracked_info->m_pipelineState;
	}

	ComPtr<ID3D12StateObject> PipelineStateBase::get_native_state()
	{
		return tracked_info->m_StateObject;
	}
	PipelineState::PipelineState(PipelineStateDesc _desc, std::string cache) : desc(_desc)
	{
		this->cache = cache;
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

	

	}
	PipelineState::ptr PipelineStateCache::get_cache(PipelineStateDesc& desc, std::string name)
	{

		desc.name = name;

		//	 return  PipelineState::ptr(new PipelineState(desc));
		return Singleton<PipelineStateCache>::get().cache[desc];
	}

	ComputePipelineState::ptr PipelineStateCache::get_cache(ComputePipelineStateDesc& desc, std::string name)
	{
		desc.name = name;
		return Singleton<PipelineStateCache>::get().compute_cache[desc];
	}

	void ComputePipelineState::on_change()
	{

		root_signature = desc.root_signature;

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

		if (desc.root_signature)
			psoDesc.pRootSignature = desc.root_signature->get_native().Get();

		if (desc.shader)
			psoDesc.CS = { reinterpret_cast<UINT8*>(const_cast<char*>(desc.shader->get_blob().data())), static_cast<UINT>(desc.shader->get_blob().size()) };


		if (!cache.empty())
		{
			psoDesc.CachedPSO.pCachedBlob = cache.c_str();
			psoDesc.CachedPSO.CachedBlobSizeInBytes = cache.size();
		}
		else
		{
			//	assert(false);
		}

		HRESULT hr = (Device::get().get_native_device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&tracked_info->m_pipelineState)));

		if (hr != S_OK)
		{
			psoDesc.CachedPSO = {};
			hr = (Device::get().get_native_device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&tracked_info->m_pipelineState)));
		}


		debuggable = desc.shader && desc.shader->depends_on("DebugInfo");

		name = desc.name;

		//	TEST(hr);

		cache.clear();

	}

	ComputePipelineState::~ComputePipelineState()
	{
		unregister_shader(desc.shader);

	}

}


BOOST_CLASS_EXPORT_IMPLEMENT(DX12::PipelineStateBase);
BOOST_CLASS_EXPORT_IMPLEMENT(DX12::PipelineState);