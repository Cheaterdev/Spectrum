

static const wchar_t* c_hitGroupName = L"MyHitGroup";
static const wchar_t* c_raygenShaderName = L"MyRaygenShader";
static const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
static const wchar_t* c_missShaderName = L"MyMissShader";


struct RayGenConstantBuffer
{
	UINT texture_offset;
	UINT node_offset;
};


class RTX :public Singleton<RTX>, Events::prop_handler,
	public Events::Runner
{
public:
	Resource::ptr m_missShaderTable;
	Resource::ptr m_hitGroupShaderTable;
	Resource::ptr m_rayGenShaderTable;

	shader_identifier rayGenShaderIdentifier;
	shader_identifier missShaderIdentifier;
	shader_identifier missShadowShaderIdentifier;


	ComPtr<ID3D12StateObject> m_dxrStateObject;
	ComPtr<ID3D12StateObject> m_SharedCollection;
	ComPtr<ID3D12StateObject> m_GlobalCollection;

	std::string blob;

	RayGenConstantBuffer m_rayGenCB;
	ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
	RootSignature::ptr global_sig;

	using ptr = std::shared_ptr<RTX>;

	//ArraysHolder<InstanceData> instanceData;

	std::set<ComPtr<ID3D12StateObject>> all_objs;

	virtual_gpu_buffer<closesthit_identifier>::ptr material_hits;// (MAX_COMMANDS_SIZE)

	std::map< materials::universal_material*, int> materials;

	bool need_recreate = false;

	std::mutex m;
	RTX();

	UINT get_material_id(materials::universal_material* universal);

	void CreateCommonProps(CD3DX12_STATE_OBJECT_DESC& raytracingPipeline);

	void CreateSharedCollection();

	void CreateGlobalCollection();


	void CreateRaytracingPipelineStateObject();


	void prepare(CommandList::ptr& list);

	void render(MeshRenderContext::ptr context, Render::TextureView& texture, Render::RaytracingAccelerationStructure::ptr scene_as);
};
