// GPU-driven replacement for VSM.cpp's old per-frame CPU
// scene->iterate_meshes() walk (Phase 5.12): for every (mesh, active+dirty
// level) pair, test the mesh's world AABB against that level's light-space
// bounds and Append() a VSMDispatchCommandData entry for every overlap.
// Modeled directly on gather_pipeline.hlsl's CS (the main camera's own
// GPU-driven command generation) -- reuses the same GatherPipelineGlobal
// (scene->compiledGather[MESH_TYPE::ALL], already built once per frame
// regardless of VSM) and SceneData bindings.
//
// Phase 5.19: CS below now skips alpha-cutout materials (routed instead to
// CS_MATERIAL's per-pipeline buckets, see VSMGatherDispatchMaterialData's
// own comment in vsm.sig) -- everything else (the common, opaque case)
// still goes through this single default list/dispatch exactly as before.

#include "autogen/GatherPipelineGlobal.h"
#include "autogen/SceneData.h"

static const GatherPipelineGlobal pip = GetGatherPipelineGlobal();

// Both VSMGatherDispatchData and VSMGatherDispatchMaterialData bind
// DefaultLayout::Instance1 -- their generated headers each claim that
// slot with a #ifndef/#error guard, so only ONE may ever be #included in a
// given compile of this file, matching which entry point (CS vs
// CS_MATERIAL) is actually being built.
#ifdef BUILD_FUNC_CS
#include "autogen/VSMGatherDispatchData.h"
static const VSMGatherDispatchData gatherData = GetVSMGatherDispatchData();
#elif defined(BUILD_FUNC_CS_MATERIAL)
#include "autogen/VSMGatherDispatchMaterialData.h"
static const VSMGatherDispatchMaterialData gatherData = GetVSMGatherDispatchMaterialData();
// Same packed-8-ids convention as meshrender.sig's GatherPipeline.pip_ids /
// gather_pipeline.hlsl's ids[8] -- see vsm_append_to_bucket below. Direct
// field access (not GetMaterial_pip_ids(), which is the per-index accessor
// generated for a fixed-size array field), matching gather_pipeline.hlsl's
// own `(uint[8])pipi.pip_ids` cast exactly.
static const uint material_ids[8] = (uint[8])gatherData.material_pip_ids;
#endif

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

// Shared by CS and CS_MATERIAL: the actual mesh-vs-level overlap test and
// VSMDispatchCommandData construction, everything except where the built
// entry ends up. Returns false (entry left zeroed) on no-overlap or a
// degenerate (zero-meshlet) mesh -- callers just skip appending.
//
// material_cb is populated here unconditionally (not just for CS_MATERIAL's
// alpha-cutout entries) -- see VSMDispatchCommandData's own comment in
// vsm.sig for why the default/opaque list needs it wired too, even though
// VSMDepthDraw's shader never reads it.
bool vsm_try_build_entry(MeshCommandData mesh, MaterialCommandData material, VSMLevelDispatchInfo level, float4x4 light_view, out VSMDispatchCommandData entry)
{
	entry = (VSMDispatchCommandData)0;

	node_data node = GetSceneData().GetNodes()[mesh.GetNode_offset()];

	float2 bmin, bmax;
	aabb_light_space_bounds(node.GetAabb(), node.GetNode_global_matrix(), light_view, bmin, bmax);

	float2 lmin = level.GetBounds_min();
	float2 lmax = level.GetBounds_max();

	if (any(bmax < lmin) || any(bmin > lmax)) return false;

	uint pair_count = mesh.GetMeshlet_count() * VSM_PAGES_PER_LEVEL;
	uint as_groups = (pair_count + 31) / 32;
	if (as_groups == 0) return false;

	entry.page_batch_cb = level.GetPage_batch_cb();
	entry.mesh_cb = mesh.GetMesh_cb();
	entry.meshinstance_cb = mesh.GetMeshinstance_cb();
	entry.material_cb = material.GetMaterial_cb();
	entry.draw_commands.counts = uint3(as_groups, 1, 1);
	return true;
}

#ifdef BUILD_FUNC_CS
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	if (dispatchID.x >= pip.GetMeshes_count()[0]) return;
	if (dispatchID.y >= gatherData.GetLevel_count()) return;

	uint id = pip.GetCommands().Load(dispatchID.x);
	MeshCommandData mesh = GetSceneData().GetMeshes()[id];

	// Alpha-cutout materials are routed to CS_MATERIAL's per-pipeline
	// buckets instead (see this file's own top comment) -- skip them here
	// so they aren't drawn twice (once opaque via this list, once properly
	// alpha-tested via theirs).
	MaterialCommandData material = GetSceneData().GetMaterials()[mesh.GetMaterial_id()];
	if (material.GetIs_transparent()) return;

	VSMLevelDispatchInfo level = gatherData.GetLevels()[dispatchID.y];

	VSMDispatchCommandData entry;
	if (!vsm_try_build_entry(mesh, material, level, gatherData.GetLight_view(), entry)) return;

	gatherData.GetDispatch_commands().Append(entry);
}
#endif

#ifdef BUILD_FUNC_CS_MATERIAL
// Routes an already-built entry into whichever of this batch's <=8 bucket
// lists matches the mesh's material pipeline -- same linear-check shape as
// gather_pipeline.hlsl's get_index. A pipeline_id not present in this
// batch (handled by a different batch, or genuinely opaque -- shouldn't
// reach here, CS_MATERIAL already checked is_transparent) silently drops,
// same as get_index's own behavior.
void vsm_append_to_bucket(uint pipeline_id, in VSMDispatchCommandData entry)
{
#define VSM_BUCKET_CHECK(x) if (material_ids[x] == pipeline_id) { gatherData.GetMaterial_commands(x).Append(entry); return; }
	VSM_BUCKET_CHECK(0)
	VSM_BUCKET_CHECK(1)
	VSM_BUCKET_CHECK(2)
	VSM_BUCKET_CHECK(3)
	VSM_BUCKET_CHECK(4)
	VSM_BUCKET_CHECK(5)
	VSM_BUCKET_CHECK(6)
	VSM_BUCKET_CHECK(7)
#undef VSM_BUCKET_CHECK
}

[numthreads(64, 1, 1)]
void CS_MATERIAL(uint3 dispatchID : SV_DispatchThreadID)
{
	if (dispatchID.x >= pip.GetMeshes_count()[0]) return;
	if (dispatchID.y >= gatherData.GetLevel_count()) return;

	uint id = pip.GetCommands().Load(dispatchID.x);
	MeshCommandData mesh = GetSceneData().GetMeshes()[id];

	MaterialCommandData material = GetSceneData().GetMaterials()[mesh.GetMaterial_id()];
	if (!material.GetIs_transparent()) return;

	VSMLevelDispatchInfo level = gatherData.GetLevels()[dispatchID.y];

	VSMDispatchCommandData entry;
	if (!vsm_try_build_entry(mesh, material, level, gatherData.GetLight_view(), entry)) return;

	vsm_append_to_bucket(material.GetPipeline_id(), entry);
}
#endif
