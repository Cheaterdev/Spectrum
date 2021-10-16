#include "pch_dx.h"
#include "PipelineState.h"
#include "Serialization/Serializer.h"

import Debug;

template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE T>
struct TypeToOBJ;

template<class T>
struct OBJToType;


#define MAP(x,y) \
template<> \
struct TypeToOBJ<x> {\
	typedef y type;\
};\
template<> \
struct OBJToType<y> {\
	static const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = x;\
};

MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, D3D12_DEPTH_STENCIL_DESC);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO, D3D12_CACHED_PIPELINE_STATE);
MAP(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT);


namespace DX12
{
	template < class V>
	struct alignas(void*) Record
	{
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type;
		V v;
	};

	class PSOCreator
	{
		std::string data;

		UINT write_offset = 0;

		void request(UINT size)
		{

			data.resize(data.size() + size);
			assert(data.size() > (write_offset + size));

		}

		template <class T>
		void push_one(const T& elem)
		{
			request(sizeof(T));
			memcpy(data.data() + write_offset, &elem, sizeof(T));
			write_offset += sizeof(T);

		}


		template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, class T>
		void internal_inluce(const T& v)
		{
			Record<T> r;
			r.type = type;
			r.v = v;

			push_one(r);
		}

		template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE T,class S>
		void include_shader(S& shader)
		{
			std::string& blob = shader->get_blob();
			D3D12_SHADER_BYTECODE code;
			code.BytecodeLength = blob.size();
			code.pShaderBytecode = reinterpret_cast<UINT8*>(const_cast<char*>(blob.data()));
			internal_inluce<T>(code);
		}
	public:


		PSOCreator()
		{
			data.reserve(65536);
		}
		template<class T>
		void include(const T&v)
		{
			internal_inluce<OBJToType<T>::type>(v);
		}


		void include(vertex_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>(shader);
		}

		void include(pixel_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>(shader);
		}

		void include(geometry_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>(shader);
		}

		void include(domain_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>(shader);
		}

		void include(hull_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>(shader);
		}


		void include(mesh_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>(shader);
		}

		void include(amplification_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>(shader);
		}


		D3D12_PIPELINE_STATE_STREAM_DESC get_desc()
		{
			D3D12_PIPELINE_STATE_STREAM_DESC desc;
			desc.SizeInBytes = data.size();
			desc.pPipelineStateSubobjectStream = data.data();
			return desc;
		}
	};

	void PipelineState::on_change()
	{
		root_signature = desc.root_signature;

		auto t = CounterManager::get().start_count<PipelineState>();

		PSOCreator creator;


		creator.include(desc.root_signature->get_native().Get());


		
		//slots.clear();
		if (desc.vertex)
		{
			creator.include(desc.vertex);
	//		slots.merge(desc.vertex->slots_usage);
		}
		if (desc.pixel)
		{
			creator.include(desc.pixel);
		//	slots.merge(desc.pixel->slots_usage);
		}
		if (desc.geometry)
		{
			creator.include(desc.geometry);
	//		slots.merge(desc.geometry->slots_usage);
		}
		if (desc.domain)
		{
			creator.include(desc.domain);
	//		slots.merge(desc.domain->slots_usage);
		}
		if (desc.hull)
		{
			creator.include(desc.hull);
		//	slots.merge(desc.hull->slots_usage);
		}
		if (desc.mesh)
		{
			creator.include(desc.mesh);
		//	slots.merge(desc.mesh->slots_usage);
		}
		if (desc.amplification)
		{
			creator.include(desc.amplification);
		//	slots.merge(desc.amplification->slots_usage);
		}
		
		{
			CD3DX12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			RasterizerState.CullMode = desc.rasterizer.cull_mode;
			RasterizerState.FillMode = desc.rasterizer.fill_mode;
			RasterizerState.ConservativeRaster = desc.rasterizer.conservative ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			creator.include((D3D12_RASTERIZER_DESC)RasterizerState);
		}

		{
			CD3DX12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			BlendState.IndependentBlendEnable = desc.blend.independ_blend;
			for (unsigned int i = 0; i < 8; i++)
			{
				BlendState.RenderTarget[i].BlendEnable = desc.blend.render_target[i].enabled;
				BlendState.RenderTarget[i].SrcBlend = desc.blend.render_target[i].source;
				BlendState.RenderTarget[i].DestBlend = desc.blend.render_target[i].dest;
				BlendState.RenderTarget[i].DestBlendAlpha = desc.blend.render_target[i].dest_alpha;
				BlendState.RenderTarget[i].SrcBlendAlpha = desc.blend.render_target[i].source_alpha;

			}
			creator.include((D3D12_BLEND_DESC)BlendState);
		}

		{
			CD3DX12_DEPTH_STENCIL_DESC DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);;
			DepthStencilState.DepthWriteMask = desc.rtv.enable_depth_write ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
			DepthStencilState.DepthEnable = desc.rtv.enable_depth;
			DepthStencilState.DepthFunc = desc.rtv.func;

			DepthStencilState.StencilEnable = desc.rtv.enable_stencil;
			DepthStencilState.StencilReadMask = desc.rtv.stencil_read_mask;
			DepthStencilState.StencilWriteMask = desc.rtv.stencil_write_mask;
			DepthStencilState.FrontFace = desc.rtv.stencil_desc;
			DepthStencilState.BackFace = desc.rtv.stencil_desc;

			creator.include((D3D12_DEPTH_STENCIL_DESC)DepthStencilState);
		}

		{
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType = desc.topology;
			creator.include(PrimitiveTopologyType);
		}

		{
			D3D12_RT_FORMAT_ARRAY rtvs;
			rtvs.NumRenderTargets = static_cast<UINT>(desc.rtv.rtv_formats.size());
unsigned int i = 0;
			for (; i < desc.rtv.rtv_formats.size(); i++)
				rtvs.RTFormats[i] = desc.rtv.rtv_formats[i];
	for (; i < 8; i++)
		rtvs.RTFormats[i] = DXGI_FORMAT_UNKNOWN;
			creator.include(rtvs);
		}

		{
			DXGI_FORMAT DSVFormat = desc.rtv.ds_format;

			creator.include(DSVFormat);
		}
	

	

		//psoDesc.SampleMask = UINT_MAX;
	//	psoDesc.SampleDesc.Count = 1;
	//	psoDesc.SampleDesc.Quality = 0;

		auto nonCached = creator.get_desc();

		if (!cache.empty())
		{
			D3D12_CACHED_PIPELINE_STATE cached;

			cached.pCachedBlob = cache.c_str();
			cached.CachedBlobSizeInBytes = cache.size();
			creator.include(cached);
		}
		

		auto cached = creator.get_desc();
		HRESULT hr = (Device::get().get_native_device()->CreatePipelineState(&cached, IID_PPV_ARGS(&tracked_info->m_pipelineState)));

		if (hr != S_OK)
		{
			hr = (Device::get().get_native_device()->CreatePipelineState(&nonCached, IID_PPV_ARGS(&tracked_info->m_pipelineState)));
		}



		cache.clear();

		debuggable = false;
		debuggable |= desc.vertex && desc.vertex->depends_on("DebugInfo");
		debuggable |= desc.pixel && desc.pixel->depends_on("DebugInfo");
		debuggable |= desc.geometry && desc.geometry->depends_on("DebugInfo");
		debuggable |= desc.domain && desc.domain->depends_on("DebugInfo");
		debuggable |= desc.hull && desc.hull->depends_on("DebugInfo");
		debuggable |= desc.mesh && desc.mesh->depends_on("DebugInfo");
		debuggable |= desc.amplification && desc.amplification->depends_on("DebugInfo");


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

		register_shader(desc.mesh);
		register_shader(desc.amplification);

	}
	PipelineState::~PipelineState()
	{
		unregister_shader(desc.pixel);
		unregister_shader(desc.vertex);
		unregister_shader(desc.hull);
		unregister_shader(desc.domain);
		unregister_shader(desc.geometry);

		unregister_shader(desc.mesh);
		unregister_shader(desc.amplification);

	}


	PipelineStateCache::~PipelineStateCache()
	{
		std::lock_guard<std::mutex> g(m);

			
		FileSystem::get().save_data("pso",Serializer::serialize(binary_cache));
	}

	PipelineStateCache::PipelineStateCache(): cache([this](const PipelineStateDesc& desc)
	{
		std::lock_guard<std::mutex> g(m);

		std::string binary = desc.name.empty()?"": binary_cache[desc.name];


		//Log::get() << desc << Log::endl;
		auto state=  PipelineState::ptr(new PipelineState(desc, binary));

		if (!desc.name.empty())
		{
			binary_cache[desc.name] = state->get_cache();
		}

		return state;

	}), compute_cache([this](const ComputePipelineStateDesc& desc)
	{
		std::lock_guard<std::mutex> g(m);

		std::string binary = desc.name.empty() ? "" : binary_cache[desc.name];


		//Log::get() << desc << Log::endl;
		auto state = ComputePipelineState::ptr(new ComputePipelineState(desc,binary));

		if (!desc.name.empty())
		{
			binary_cache[desc.name] = state->get_cache();
		}

		return state;

	})
	{

		std::lock_guard<std::mutex> g(m);

		auto file = FileSystem::get().get_file("pso");
		if(file)
			Serializer::deserialize(file->load_all(), binary_cache);
			
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