// GPU-driven replacement for VSM.cpp's old per-frame CPU
// scene->iterate_meshes() walk (Phase 5.12): for every (mesh, active+dirty
// level) pair, test the mesh's world AABB against that level's light-space
// bounds and Append() a VSMDispatchCommandData entry for every overlap.
// Modeled directly on gather_pipeline.hlsl's CS (the main camera's own
// GPU-driven command generation) -- reuses the same GatherPipelineGlobal
// (scene->compiledGather[MESH_TYPE::ALL], already built once per frame
// regardless of VSM) and SceneData bindings, simplified since VSM is
// depth-only (no per-material pipeline routing, no material lookup).

#include "autogen/GatherPipelineGlobal.h"
#include "autogen/SceneData.h"
#include "autogen/VSMGatherDispatchData.h"

static const GatherPipelineGlobal pip = GetGatherPipelineGlobal();
static const VSMGatherDispatchData gatherData = GetVSMGatherDispatchData();

// 4x4 pages per level, matching VSM.cpp / VSMClipmap / mesh_shader_vsm.hlsl.
#define VSM_PAGES_PER_LEVEL 16

float3 local_to_world(float3 local, float4x4 mat)
{
	float4 res = mul(mat, float4(local, 1));
	return res.xyz / res.w;
}

// Transforms the AABB's 8 corners through node_mat then light_view, and
// returns the light-space XY bounding rect -- same "transform 8 corners,
// take the bounds" approach VSM.cpp's CPU invalidation code already uses
// (make_light_view_camera + get_points/get_bounds_in), just done on GPU.
void aabb_light_space_bounds(AABB aabb, float4x4 node_mat, float4x4 light_view, out float2 bmin, out float2 bmax)
{
	float3 mn = aabb.GetMin().xyz;
	float3 mx = aabb.GetMax().xyz;

	float3 corners[8] = {
		float3(mn.x, mn.y, mn.z), float3(mx.x, mn.y, mn.z),
		float3(mn.x, mx.y, mn.z), float3(mx.x, mx.y, mn.z),
		float3(mn.x, mn.y, mx.z), float3(mx.x, mn.y, mx.z),
		float3(mn.x, mx.y, mx.z), float3(mx.x, mx.y, mx.z),
	};

	bmin = float2(1e20, 1e20);
	bmax = float2(-1e20, -1e20);

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		float3 wpos = local_to_world(corners[i], node_mat);
		float3 lpos = local_to_world(wpos, light_view);
		bmin = min(bmin, lpos.xy);
		bmax = max(bmax, lpos.xy);
	}
}

[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	if (dispatchID.x >= pip.GetMeshes_count()[0]) return;
	if (dispatchID.y >= gatherData.GetLevel_count()) return;

	uint id = pip.GetCommands().Load(dispatchID.x);
	MeshCommandData mesh = GetSceneData().GetMeshes()[id];
	node_data node = GetSceneData().GetNodes()[mesh.GetNode_offset()];

	float2 bmin, bmax;
	aabb_light_space_bounds(node.GetAabb(), node.GetNode_global_matrix(), gatherData.GetLight_view(), bmin, bmax);

	VSMLevelDispatchInfo level = gatherData.GetLevels()[dispatchID.y];
	float2 lmin = level.GetBounds_min();
	float2 lmax = level.GetBounds_max();

	if (any(bmax < lmin) || any(bmin > lmax)) return;

	uint pair_count = mesh.GetMeshlet_count() * VSM_PAGES_PER_LEVEL;
	uint as_groups = (pair_count + 31) / 32;
	if (as_groups == 0) return;

	VSMDispatchCommandData entry;
	entry.page_batch_cb = level.GetPage_batch_cb();
	entry.mesh_cb = mesh.GetMesh_cb();
	entry.meshinstance_cb = mesh.GetMeshinstance_cb();
	entry.draw_commands.counts = uint3(as_groups, 1, 1);

	gatherData.GetDispatch_commands().Append(entry);
}
