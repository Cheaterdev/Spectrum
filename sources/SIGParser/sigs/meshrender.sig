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

[Bind = DefaultLayout::Instance2]
struct MeshInstanceInfo
{
	StructuredBuffer<mesh_vertex_input> vertexes;
	StructuredBuffer<uint> indices;
	StructuredBuffer<Meshlet> meshlets;

	StructuredBuffer<MeshletCullData> meshletCullData;

	StructuredBuffer<uint> unique_indices;
	StructuredBuffer<uint> primitive_indices;

}


[Bind = DefaultLayout::Instance1]
struct MeshInfo
{
    uint vertex_offset_local;
	uint meshlet_offset_local;

    uint node_offset;
	uint meshlet_count;
}


struct RaytraceInstanceInfo
{
	StructuredBuffer<mesh_vertex_input> vertexes;
	StructuredBuffer<uint> indices;	
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



[IndirectCommand]
[shader_only]
struct DrawIndexedArguments
{
	uint data[5];
}

[IndirectCommand]
[shader_only]
struct DispatchMeshArguments
{
	uint3 counts;
}

[IndirectCommand]
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

[IndirectCommand]
struct CommandData
{
	MeshInfo* mesh_cb;
	MeshInstanceInfo* meshinstance_cb;
	MaterialInfo* material_cb;
	DispatchMeshArguments draw_commands;
}

struct MeshCommandData
{
	uint material_id;
	uint node_offset;
	MeshInfo* mesh_cb;
	MeshInstanceInfo* meshinstance_cb;
	DispatchMeshArguments draw_commands;
	uint meshlet_count;
}

struct MaterialCommandData
{
	uint material_cb;
	uint pipeline_id;
	# Set from universal_material::is_transparent() (the material graph's
	# opacity output is actually driven) at the same site material_cb/
	# pipeline_id are populated. Lets a GPU-side gather pass (e.g. VSM's
	# vsm_gather_dispatch.hlsl) route a mesh differently -- alpha-cutout
	# materials need a depth pass with a real pixel shader (clip()), opaque
	# ones don't -- without decoding the material graph itself.
	uint is_transparent;
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

# Producers maintain counters manually (InterlockedAdd) instead of Append so
# the slot index is known: it initializes visible_boxes[idx] (kills the old
# full-buffer 999 clears) and bumps the consumer's DispatchArguments /
# DrawIndexedArguments via InterlockedMax (kills the InitDispatch dispatches
# and the counter->draw-args copy).
[Bind = DefaultLayout::Instance1]
struct GatherBoxes
{
	RWStructuredBuffer<BoxInfo> culledMeshes;
	RWStructuredBuffer<uint> culledCount;
	RWStructuredBuffer<uint> visible_boxes;
	RWStructuredBuffer<DrawIndexedArguments> drawBoxesArgs;
	RWStructuredBuffer<DispatchArguments> gatherMeshesArgs;

	RWStructuredBuffer<uint> visibleMeshes;
	RWStructuredBuffer<uint> visibleCount;
	RWStructuredBuffer<DispatchArguments> renderArgs;
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

	RWStructuredBuffer<uint> visibleMeshes;
	RWStructuredBuffer<uint> visibleCount;
	RWStructuredBuffer<DispatchArguments> renderArgs;

	RWStructuredBuffer<uint> invisibleMeshes;
	RWStructuredBuffer<uint> invisibleCount;
	RWStructuredBuffer<DispatchArguments> retestArgs;
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

# Per-stage arg resets are plain clear_uav-to-zero calls (CLEAR sync differs
# from COMPUTE, so the state tracker emits a real barrier before the producer
# dispatches — a reset CS would be same-state UAV->UAV and race). Producers
# restore the constant components (dispatch y/z = 1, draw IndexCount = 36)
# with benign racing writes whenever they bump the counts.



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

	# reversed-Z: box passes if any pixel is at least as close as the farthest
	# occluder depth stored in the HiZ tile (see downsample_depth.hlsl min()).
	depth_func = GREATER_EQUAL;
}
