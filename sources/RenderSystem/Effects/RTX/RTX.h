

struct RayType
{
	bool per_hit_id;

	std::string group_name;
//	shader_identifier raygen;
	shader_identifier miss;
	
	std::optional<shader_identifier> hit;


//	std::string raygen_name;
	std::string miss_name;
	std::string hit_name;
};


struct RayGenShader
{
	std::string name;

	shader_identifier raygen;

};
struct RayGenConstantBuffer
{
	UINT texture_offset;
	UINT node_offset;
};

struct HitObject
{
	
};
class RTX :public Singleton<RTX>, Events::prop_handler,
	public Events::Runner
{
	
public:
	Resource::ptr m_missShaderTable;
	Resource::ptr m_hitGroupShaderTable;
	Resource::ptr m_rayGenShaderTable;

	StateObject::ptr m_dxrStateObject;
	StateObject::ptr m_SharedCollection;

	library_shader::ptr library;

	RayGenConstantBuffer m_rayGenCB;
	RootSignature::ptr global_sig;
	Render::RootSignature::ptr local_sig;
	using ptr = std::shared_ptr<RTX>;


	std::vector<RayType> ray_types;
	std::vector<RayGenShader> raygen_types;
	
	//ArraysHolder<InstanceData> instanceData;

	std::set<ComPtr<ID3D12StateObject>> all_objs;

	virtual_gpu_buffer<closesthit_identifier>::ptr material_hits;// (MAX_COMMANDS_SIZE)

	std::map< materials::universal_material*, UINT> materials;

	bool need_recreate = false;

	std::mutex m;
	RTX();
	TypedHandle<closesthit_identifier> allocate_hit()
	{
		return material_hits->allocate(ray_types.size());
	}
	UINT get_material_id(materials::universal_material* universal);

	void CreateCommonProps(StateObjectDesc& desc);

	void CreateSharedCollection();

	StateObject::ptr CreateGlobalCollection(materials::universal_material* mat);


	void CreateRaytracingPipelineStateObject();


	void prepare(CommandList::ptr& list);

	void render(ComputeContext& compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec3 size);
};
