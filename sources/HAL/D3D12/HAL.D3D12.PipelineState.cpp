module HAL:PipelineState;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;


#undef THIS


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
			assert(data.size() >= (write_offset + size));

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

		template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE T, class S>
		void include_shader(S& shader)
		{
			auto& blob = shader->get_blob();
			D3D12_SHADER_BYTECODE code;
			code.BytecodeLength = blob.size();
			code.pShaderBytecode = blob.data();
			internal_inluce<T>(code);
		}
	public:


		PSOCreator()
		{
			data.reserve(65536);
		}
		template<class T>
		void include(const T& v)
		{
			internal_inluce<OBJToType<T>::type>(v);
		}


		void include(HAL::vertex_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>(shader);
		}

		void include(HAL::pixel_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>(shader);
		}

		void include(HAL::geometry_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>(shader);
		}

		void include(HAL::domain_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>(shader);
		}

		void include(HAL::hull_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>(shader);
		}


		void include(HAL::mesh_shader::ptr shader)
		{
			include_shader<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>(shader);
		}

		void include(HAL::amplification_shader::ptr shader)
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


namespace HAL
{
	namespace API
	{
		ComPtr<ID3D12PipelineState> PipelineStateBase::get_native()
		{
			auto THIS = static_cast<HAL::PipelineStateBase*>(this);
			return THIS->tracked_info->m_pipelineState;
		}

		ComPtr<ID3D12StateObject> PipelineStateBase::get_native_state()
		{
			auto THIS = static_cast<HAL::PipelineStateBase*>(this);
			return THIS->tracked_info->m_StateObject;
		}


		std::string PipelineStateBase::get_cache()
		{

			auto THIS = static_cast<HAL::PipelineStateBase*>(this);


			ComPtr<ID3DBlob> blob;
			THIS->tracked_info->m_pipelineState->GetCachedBlob(&blob);
			std::string str((char*)blob->GetBufferPointer(), blob->GetBufferSize());

			return str;
		}



	}



	HAL::shader_identifier StateObject::get_shader_id(std::wstring_view name)
	{
		return identify(stateObjectProperties->GetShaderIdentifier(name.data()));
	}
	void StateObject::on_change()
	{
		tracked_info.reset(new API::TrackedPipeline());
		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ desc.collection ? D3D12_STATE_OBJECT_TYPE_COLLECTION : D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		for (auto& l : desc.libraries)
		{

			auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)l.library->get_blob().data(), l.library->get_blob().size());
			lib->SetDXILLibrary(&libdxil);

			for (auto& e : l.exports)
			{

				lib->DefineExport(e.first.c_str(), e.second.empty() ? nullptr : e.second.c_str());
			}

			debuggable |= l.library && l.library->depends_on("DebugInfo");
		}

		if (desc.global_root)
		{
			auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();

			globalRootSignature->SetRootSignature(desc.global_root->get_native().Get());
		}

		if (desc.MaxTraceRecursionDepth)
		{
			auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
			pipelineConfig->Config(desc.MaxTraceRecursionDepth);
		}

		if (desc.MaxPayloadSizeInBytes)
		{
			auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
			shaderConfig->Config(desc.MaxPayloadSizeInBytes, desc.MaxAttributeSizeInBytes);
		}


		std::map<RootSignature::ptr, CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT*> roots;


		for (auto& e : desc.hit_groups)
		{
			if (e.local_root)
				roots[e.local_root] = nullptr;
		}


		for (auto& e : roots)
		{
			auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			localRootSignature->SetRootSignature(e.first->get_native().Get());

			auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);

			e.second = rootSignatureAssociation;
		}


		for (auto& e : desc.hit_groups)
		{
			auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();

			hitGroup->SetHitGroupExport(e.name.c_str());
			//TODO: more shaders
			hitGroup->SetClosestHitShaderImport(e.closest_hit_shader.c_str());
			hitGroup->SetHitGroupType(to_native(e.type));

			if (e.local_root)
				roots[e.local_root]->AddExport(e.name.c_str());
		}

		for (auto& c : desc.collections)
		{
			auto sharedCollection = raytracingPipeline.CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
			sharedCollection->SetExistingCollection(c->get_native_state().Get());

			debuggable |= c->debuggable;
		}


		TEST(desc.global_root->get_device(), desc.global_root->get_device().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&tracked_info->m_StateObject)));
		TEST(desc.global_root->get_device(), tracked_info->m_StateObject.As(&stateObjectProperties));
		assert(stateObjectProperties);
		event_change();

	}

	void PipelineState::on_change()
	{
		tracked_info.reset(new API::TrackedPipeline());
		root_signature = desc.root_signature;

	
		PSOCreator creator;


		creator.include(desc.root_signature->get_native().Get());



		slots.clear();
		if (desc.vertex)
		{
			creator.include(desc.vertex);
			slots.merge(desc.vertex->slots_usage);
		}
		if (desc.pixel)
		{
			creator.include(desc.pixel);
			slots.merge(desc.pixel->slots_usage);
		}
		if (desc.geometry)
		{
			creator.include(desc.geometry);
			slots.merge(desc.geometry->slots_usage);
		}
		if (desc.domain)
		{
			creator.include(desc.domain);
			slots.merge(desc.domain->slots_usage);
		}
		if (desc.hull)
		{
			creator.include(desc.hull);
			slots.merge(desc.hull->slots_usage);
		}
		if (desc.mesh)
		{
			creator.include(desc.mesh);
			slots.merge(desc.mesh->slots_usage);
		}
		if (desc.amplification)
		{
			creator.include(desc.amplification);
			slots.merge(desc.amplification->slots_usage);
		}

		assert(!slots.empty());
		{
			CD3DX12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			RasterizerState.CullMode = to_native(desc.rasterizer.cull_mode);
			RasterizerState.FillMode = to_native(desc.rasterizer.fill_mode);
			RasterizerState.ConservativeRaster = desc.rasterizer.conservative ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			creator.include((D3D12_RASTERIZER_DESC)RasterizerState);
		}

		{
			CD3DX12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			BlendState.IndependentBlendEnable = desc.blend.independ_blend;
			for (unsigned int i = 0; i < 8; i++)
			{
				BlendState.RenderTarget[i].BlendEnable = desc.blend.render_target[i].enabled;
				BlendState.RenderTarget[i].SrcBlend = to_native(desc.blend.render_target[i].source);
				BlendState.RenderTarget[i].DestBlend = to_native(desc.blend.render_target[i].dest);
				BlendState.RenderTarget[i].DestBlendAlpha = to_native(desc.blend.render_target[i].dest_alpha);
				BlendState.RenderTarget[i].SrcBlendAlpha = to_native(desc.blend.render_target[i].source_alpha);

			}
			creator.include((D3D12_BLEND_DESC)BlendState);
		}

		{
			CD3DX12_DEPTH_STENCIL_DESC DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);;
			DepthStencilState.DepthWriteMask = desc.rtv.enable_depth_write ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
			DepthStencilState.DepthEnable = desc.rtv.enable_depth;
			DepthStencilState.DepthFunc = to_native(desc.rtv.func);

			DepthStencilState.StencilEnable = desc.rtv.enable_stencil;
			DepthStencilState.StencilReadMask = desc.rtv.stencil_read_mask;
			DepthStencilState.StencilWriteMask = desc.rtv.stencil_write_mask;
			DepthStencilState.FrontFace = to_native(desc.rtv.stencil_desc);
			DepthStencilState.BackFace = to_native(desc.rtv.stencil_desc);

			creator.include((D3D12_DEPTH_STENCIL_DESC)DepthStencilState);
		}

		{
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType = to_native(desc.topology);
			creator.include(PrimitiveTopologyType);
		}

		{
			D3D12_RT_FORMAT_ARRAY rtvs;
			rtvs.NumRenderTargets = static_cast<UINT>(desc.rtv.rtv_formats.size());
			unsigned int i = 0;
			for (; i < desc.rtv.rtv_formats.size(); i++)
				rtvs.RTFormats[i] = to_native(desc.rtv.rtv_formats[i]);
			for (; i < 8; i++)
				rtvs.RTFormats[i] = DXGI_FORMAT_UNKNOWN;
			creator.include(rtvs);
		}

		{
			DXGI_FORMAT DSVFormat = to_native(desc.rtv.ds_format);

			creator.include(DSVFormat);
		}




		//psoDesc.SampleMask = UINT_MAX;
	//	psoDesc.SampleDesc.Count = 1;
	//	psoDesc.SampleDesc.Quality = 0;

		auto nonCached = creator.get_desc();


	/*	static std::mutex m;
		std::lock_guard<std::mutex> g(m);*/
		if (!cache.empty())
		{
			D3D12_CACHED_PIPELINE_STATE cached;

			cached.pCachedBlob = cache.c_str();
			cached.CachedBlobSizeInBytes = cache.size();
			creator.include(cached);
		}else
		Log::get()<<"Creating New PSO: " << desc.name<<Log::endl;

		auto cached = creator.get_desc();
		HRESULT hr = (root_signature->get_device().get_native_device()->CreatePipelineState(&cached, IID_PPV_ARGS(&tracked_info->m_pipelineState)));

		if (hr != S_OK)
		{
			hr = (root_signature->get_device().get_native_device()->CreatePipelineState(&nonCached, IID_PPV_ARGS(&tracked_info->m_pipelineState)));
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



	void ComputePipelineState::on_change()
	{
		tracked_info.reset(new API::TrackedPipeline());

		root_signature = desc.root_signature;

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

		if (desc.root_signature)
			psoDesc.pRootSignature = desc.root_signature->get_native().Get();

		slots.clear();
		if (desc.shader)
		{
			psoDesc.CS = { desc.shader->get_blob().data(), static_cast<UINT>(desc.shader->get_blob().size()) };
			slots.merge(desc.shader->slots_usage);
		}
			assert(!slots.empty());

		if (!cache.empty())
		{
			psoDesc.CachedPSO.pCachedBlob = cache.c_str();
			psoDesc.CachedPSO.CachedBlobSizeInBytes = cache.size();
		}
		else
		{
			//	assert(false);
		}

		HRESULT hr = (root_signature->get_device().get_native_device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&tracked_info->m_pipelineState)));

		if (hr != S_OK)
		{
			psoDesc.CachedPSO = {};
			hr = (root_signature->get_device().get_native_device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&tracked_info->m_pipelineState)));
		}


		debuggable = desc.shader && desc.shader->depends_on("DebugInfo");

		name = desc.name;

		//	TEST(hr);

		cache.clear();

	}

}