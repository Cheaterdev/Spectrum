[nobind]
[serialize]
struct mesh_vertex_input
{
    float3 pos;
    float3 normal;
    float2 tc;
    float4 tangent;
}


struct AABB
{
    float4 min;
    float4 max;
}

struct node_data
{
    float4x4 node_global_matrix;
	float4x4 node_global_matrix_prev;
    float4x4 node_inverse_matrix;
    AABB aabb;
}



[Bind = DefaultLayout::Instance1]
struct MeshInfo
{
	#StructuredBuffer<node_data> nodes;
	#StructuredBuffer<mesh_vertex_input> vertexes;
	#StructuredBuffer<uint> indices;
	#StructuredBuffer<Meshlet> meshlets;

    uint node_offset;
    uint vertex_offset;
	uint meshlet_offset;
	uint meshlet_count;
	uint meshlet_unique_offset;
	uint meshlet_vertex_offset;
}

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
}

[serialize]
struct MeshletCullData
{
    float4 BoundingSphere;
    uint   NormalCone;
    float  ApexOffset;
}



[shader_only]
struct DrawIndexedArguments
{
	uint data[5];
}

[shader_only]
struct DispatchMeshArguments
{
	uint3 counts;
}

[shader_only]
struct DispatchArguments
{
	uint3 counts;
}

[shader_only]
struct GPUAddress
{
	uint2 data;
}


struct MeshInstance
{
	uint vertex_offset;
	uint index_offset;
}

struct CommandData
{
	uint mesh_cb;
	uint material_cb;
	DispatchMeshArguments draw_commands;
}

struct MeshCommandData
{
	uint material_id;
	uint node_offset;
	uint mesh_cb;
	DispatchMeshArguments draw_commands;
}

struct MaterialCommandData
{
	uint material_cb;
	uint pipeline_id;
}

[Bind = DefaultLayout::Instance0]
struct GatherPipelineGlobal
{
	StructuredBuffer<uint> meshes_count;
	Buffer<uint> commands;

}

[Bind = DefaultLayout::Instance1]
struct GatherPipeline
{
	uint4 pip_ids[2]; # PACK TO!!!! uint4 pip_ids[2];
	AppendStructuredBuffer<CommandData> commands[8];
}

struct BoxInfo
{
	uint node_offset;
	uint mesh_id;
}

[Bind = DefaultLayout::Instance1]
struct GatherBoxes
{
	AppendStructuredBuffer<BoxInfo> culledMeshes;

	AppendStructuredBuffer<uint> visibleMeshes;
}



[Bind = DefaultLayout::Instance1]
struct DrawBoxes
{
	StructuredBuffer<float4> vertices;
	StructuredBuffer<BoxInfo> input_meshes;
	
	RWStructuredBuffer<uint> visible_meshes;
}

[Bind = DefaultLayout::Instance1]
struct InitDispatch
{
	RWStructuredBuffer<uint> counter;	
	#RWStructuredBuffer<GatherPipelineGlobal> pipelineGlobal;
	RWStructuredBuffer<DispatchArguments> dispatch_data;
}

[Bind = DefaultLayout::Instance1]
struct GatherMeshesBoxes
{
	StructuredBuffer<BoxInfo> input_meshes;	
	StructuredBuffer<uint> visible_boxes;	

	AppendStructuredBuffer<uint> visibleMeshes;
	AppendStructuredBuffer<uint> invisibleMeshes;
}





ComputePSO GatherPipeline
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = gather_pipeline;

	[rename = CHECK_FRUSTUM]
	[CS, nullable]
	define CheckFrustum;
}



ComputePSO GatherBoxes
{
	root = DefaultLayout;

	[EntryPoint = CS_boxes]
	compute = gather_pipeline;

	[rename = CHECK_FRUSTUM]
	[CS, nullable]
	define CheckFrustum;
}

ComputePSO InitDispatch
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = occluder_cs_dispatch_init;

	[rename = CHECK_FRUSTUM]
	[CS, nullable]
	define CheckFrustum;
}


ComputePSO GatherMeshes
{
	root = DefaultLayout;

	[EntryPoint = CS_meshes_from_boxes]
	compute = gather_pipeline;

	[rename = INVISIBLE]
	[CS, nullable]
	define Invisible;
}



GraphicsPSO RenderBoxes
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = occluder;

	[EntryPoint = PS]
	pixel = occluder;

	conservative = true;
	depth_write = false;

	ds = D32_FLOAT;

	cull = None;

	depth_func = ALWAYS;
}
