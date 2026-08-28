// VSM-only mesh shader: renders one clipmap level's worth of pages (up to
// pages_per_level^2, routed via SV_RenderTargetArrayIndex) in a single
// DispatchMesh call per node, instead of one CPU-issued draw per page.
//
// Deliberately independent from mesh_shader.hlsl (used by GBuffer/PSSM) --
// no shared file is touched, so this cannot regress either of those.
//
// Phase 1b compaction: the CPU dispatches AS threadgroups covering
// meshlet_count * VSM_PAGES_PER_LEVEL (meshlet, page) pairs. The AS culls
// each pair against that page's own camera (bounding sphere vs frustum --
// see vsm_is_visible) and wave-compacts survivors into a payload, so an
// invisible pair never launches a mesh-shader threadgroup at all. The mesh
// shader itself no longer needs to know about culling -- every group it
// gets IS visible, by construction.

#include "Common.hlsl"

#include "autogen/MeshInfo.h"
#include "autogen/SceneData.h"
#include "autogen/MeshInstanceInfo.h"
#include "autogen/VSMPageBatch.h"
#include "autogen/VSMPageTableData.h"
#include "autogen/VSMPageHiZ.h"

static const MeshInfo meshInfo = GetMeshInfo();
static const SceneData sceneData = GetSceneData();
static const MeshInstanceInfo meshInstanceInfo = GetMeshInstanceInfo();

// pages_per_level side/total, matching VSM.cpp / VSMClipmap. No longer
// bound by the 16-viewport cap (routing is by array slice), but must stay
// in step with the C++ side.
#define VSM_PAGES_PER_LEVEL_SIDE 4
#define VSM_PAGES_PER_LEVEL (VSM_PAGES_PER_LEVEL_SIDE * VSM_PAGES_PER_LEVEL_SIDE)

// Matches VSMPageTable::VSM_INVALID_SLOT.
#define VSM_INVALID_SLOT 0xFFFFFFFFu

// Real allocator (Phase 5.3): a local page's physical slot is no longer a
// fixed function of (level, local) -- look it up in the same indirection
// texture VSM_impl.hlsl's get_shadow_vsm samples from.
uint vsm_slot_of(uint pageLocal)
{
	uint2 pageXY = uint2(pageLocal % VSM_PAGES_PER_LEVEL_SIDE, pageLocal / VSM_PAGES_PER_LEVEL_SIDE);
	return GetVSMPageTableData().GetPage_table()[uint3(pageXY, (uint)GetVSMPageBatch().GetLevel())];
}

// Phase 5.19: wpos/tc are only ever CONSUMED by VSMDepthDrawMaterial's real
// pixel shader (UniversalMaterial.hlsl's PS_VSM_DEPTH, which needs a world
// position + texcoord to sample the material's opacity texture and clip())
// -- VSMDepthDraw's own [Erase]d null pixel shader never reads them. Kept
// unconditional here (not gated behind a per-PSO define) rather than
// maintaining two mesh-shader output variants: both PSOs share this one
// file/entry point, and the extra 20 bytes/vertex is cheap next to the
// actual cost of this pass (real per-page geometry submission).
struct vsm_vertex_output
{
	float4 pos : SV_POSITION;
	float3 wpos : POSITION;
	float2 tc : TEXCOORD;
};

struct vsm_prim_attrs
{
	// Array slice, not viewport: the atlas is one slice per page, which also
	// lifts the D3D12 16-viewport-per-draw cap.
	uint slice : SV_RenderTargetArrayIndex;
};

struct Payload
{
	uint PackedPairIndices[32];
};

float vsm_plane_dist(float4 plane, float3 pt)
{
	return dot(pt, plane.xyz) + plane.w;
}

// Bounding-sphere-vs-frustum test only -- deliberately NOT mesh_shader.hlsl's
// full IsVisible(), which also does a normal-cone backface test. That test
// discards meshlets whose triangles are entirely back-facing relative to the
// camera -- correct for normal front-face rendering, but VSMDepthDraw uses
// cull=Front (renders back faces, standard shadow-map peter-panning fix), so
// the all-backface meshlets the cone test throws away are exactly the ones
// this pass needs to keep. Frustum culling alone is still valid regardless
// of winding/cull mode.
bool vsm_is_visible(MeshletCullData c, float4x4 world, Camera camera)
{
	float4 BoundingSphere = c.GetBoundingSphere();

	float3 scales = float3(
		length(float3(world[0].x, world[1].x, world[2].x)),
		length(float3(world[0].y, world[1].y, world[2].y)),
		length(float3(world[0].z, world[1].z, world[2].z)));
	float scale = max(scales.x, max(scales.y, scales.z));

	float4 center = mul(world, float4(BoundingSphere.xyz, 1));
	center.xyz /= center.w;
	float radius = BoundingSphere.w * scale;

	[unroll]
	for (int i = 0; i < 6; ++i)
	{
		float d = vsm_plane_dist(camera.GetFrustum().GetPlanes(i), center.xyz);
		if (d < -radius)
			return false;
	}

	return true;
}

// Occlusion test against this page's own Hi-Z pyramid, from the last time
// it rendered (single-phase: no same-frame retest, unlike the main camera --
// see VSMPageBatch::skip_occlusion for why that's safe). Sphere-only (no
// per-meshlet AABB exists), page cameras are orthographic so the screen
// footprint is depth-independent.
bool vsm_is_occluded(MeshletCullData c, float4x4 world, Camera page_cam, int slot)
{
	float4 BoundingSphere = c.GetBoundingSphere();

	float3 scales = float3(
		length(float3(world[0].x, world[1].x, world[2].x)),
		length(float3(world[0].y, world[1].y, world[2].y)),
		length(float3(world[0].z, world[1].z, world[2].z)));
	float scale = max(scales.x, max(scales.y, scales.z));

	float4 center = mul(world, float4(BoundingSphere.xyz, 1));
	center.xyz /= center.w;
	float radius = BoundingSphere.w * scale;

	float3 forward = page_cam.GetDirection().xyz;
	float3 near_world = center.xyz - forward * radius;

	float4 near_clip = mul(page_cam.GetViewProj(), float4(near_world, 1));
	near_clip.xyz /= near_clip.w;

	float4 center_clip = mul(page_cam.GetViewProj(), float4(center.xyz, 1));
	center_clip.xyz /= center_clip.w;

	// The sphere's actual NDC bounding rect, from projecting +-radius along
	// BOTH screen-perpendicular axes -- not just one direction assumed
	// isotropic and not just the center point. Thin/sparse geometry (a
	// fence, where the bounding sphere is mostly empty space between bars)
	// needs the whole measured footprint checked, or the nearest-point
	// sample can land somewhere unrepresentative and wrongly cull geometry
	// that's genuinely in front.
	float3 world_up = (abs(forward.y) > 0.99) ? float3(1, 0, 0) : float3(0, 1, 0);
	float3 right = normalize(cross(world_up, forward));
	float3 up = normalize(cross(forward, right));

	float2 ndc_min = center_clip.xy;
	float2 ndc_max = center_clip.xy;
	float3 offsets[4] = { right * radius, -right * radius, up * radius, -up * radius };
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		float4 p = mul(page_cam.GetViewProj(), float4(center.xyz + offsets[i], 1));
		p.xyz /= p.w;
		ndc_min = min(ndc_min, p.xy);
		ndc_max = max(ndc_max, p.xy);
	}

	// Same NDC->page-UV convention as get_shadow_vsm() in VSM_impl.hlsl.
	// NDC->UV flips Y, so min/max swap; re-sort after converting.
	float2 uv_a = ndc_min * float2(0.5, -0.5) + float2(0.5, 0.5);
	float2 uv_b = ndc_max * float2(0.5, -0.5) + float2(0.5, 0.5);
	float2 page_uv_min = min(uv_a, uv_b);
	float2 page_uv_max = max(uv_a, uv_b);

	// Entirely outside this page: let the frustum test's own result stand.
	if (any(page_uv_max < 0) || any(page_uv_min > 1))
		return false;

	// Phase 5.18 Part A: page_hiz is now float2 (.x = min/farthest, .y =
	// max/closest -- see VSMPageHiZ's own comment). This test only ever
	// used the min/farthest channel and still does; unchanged behavior.
	Texture2DArray<float2> pyramid = GetVSMPageHiZ().GetPage_hiz();
	uint pw, ph, elems, numLevels;
	pyramid.GetDimensions(0, pw, ph, elems, numLevels);

	// Standard HZB test: pick the LOD where the rect spans at most 2x2
	// texels, then sample its four corners. Constant 4 taps, no loop.
	float2 rect_texels = (page_uv_max - page_uv_min) * float2(pw, ph);
	float  mip_f = ceil(log2(max(max(rect_texels.x, rect_texels.y), 1.0) * 0.5));
	uint   mip = (uint)clamp(mip_f, 0.0, (float)(numLevels - 1));

	// MIN, not max -- see IsOccludedHiZ in mesh_shader.hlsl. Each texel holds
	// the farthest depth in its footprint, so occlusion requires being behind
	// every covered texel: near_z < min(taps).
	float sampled = 1.0;
	sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float3(page_uv_min.x, page_uv_min.y, (float)slot), mip).x);
	sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float3(page_uv_max.x, page_uv_min.y, (float)slot), mip).x);
	sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float3(page_uv_min.x, page_uv_max.y, (float)slot), mip).x);
	sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float3(page_uv_max.x, page_uv_max.y, (float)slot), mip).x);

	return near_clip.z < sampled;
}

#ifdef BUILD_FUNC_AS
groupshared Payload s_Payload;

// One thread per (meshlet, page) pair, 32 pairs per group -- flattens the
// 2D (meshlet x page) space into 1D for wave-prefix compaction, same
// technique as mesh_shader.hlsl's AS just over a bigger flattened index
// space. NOTE: like that AS, this assumes the 32-thread group fits one
// wave (true on wave32/64 hardware).
[NumThreads(32, 1, 1)]
void AS(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID)
{
	uint totalPairs = meshInfo.GetMeshlet_count() * VSM_PAGES_PER_LEVEL;
	bool valid = dtid < totalPairs;
	uint safePair = valid ? dtid : 0;

	uint meshletIndex = safePair / VSM_PAGES_PER_LEVEL;
	uint pageLocal = safePair % VSM_PAGES_PER_LEVEL;

	uint slot = vsm_slot_of(pageLocal);
	valid = valid && slot != VSM_INVALID_SLOT;
	uint safeSlot = valid ? slot : 0;

	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[safeSlot];
	node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
	matrix node_mat = node.GetNode_global_matrix();
	MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + meshletIndex];

	// Phase 2 per-page caching: a page outside this frame's dirty mask keeps
	// its existing content even if this meshlet would otherwise be visible
	// there -- CPU only clears the pages actually being re-rendered, so
	// skipping the geometry here is what makes that clear valid.
	bool page_dirty = (GetVSMPageBatch().GetDirty_mask() >> pageLocal) & 1;
	bool visible = valid && page_dirty && vsm_is_visible(cull_data, node_mat, page_cam);

	// Per-page now: a page whose slot/content survived a recenter keeps its
	// Hi-Z history even when a sibling page (freshly allocated, or every
	// page on a light move) doesn't -- see VSMPageBatch::skip_occlusion.
	bool skip_occlusion = (GetVSMPageBatch().GetSkip_occlusion() >> pageLocal) & 1;
	if (visible && !skip_occlusion)
		visible = !vsm_is_occluded(cull_data, node_mat, page_cam, safeSlot);

	if (visible)
	{
		uint index = WavePrefixCountBits(visible);
		s_Payload.PackedPairIndices[index] = dtid;
	}

	uint visibleCount = WaveActiveCountBits(visible);
	DispatchMesh(visibleCount, 1, 1, s_Payload);
}
#endif

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void VS(
	uint gtid : SV_GroupThreadID,
	uint gid : SV_GroupID,
	in payload Payload payload,
	out indices uint3 tris[64],
	out vertices vsm_vertex_output verts[128],
	out primitives vsm_prim_attrs prims[64]
)
{
	uint pairIndex = payload.PackedPairIndices[gid];
	uint meshletIndex = pairIndex / VSM_PAGES_PER_LEVEL;
	uint pageLocal = pairIndex % VSM_PAGES_PER_LEVEL;

	// AS only ever forwards pairs that passed its own INVALID_SLOT check, so
	// this lookup is guaranteed valid here.
	uint slot = vsm_slot_of(pageLocal);
	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[slot];
	node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
	matrix node_mat = node.GetNode_global_matrix();

	Meshlet m = meshInstanceInfo.GetMeshlets()[meshInfo.GetMeshlet_offset_local() + meshletIndex];
	SetMeshOutputCounts(m.GetVertexCount(), m.GetPrimitiveCount());

	if (gtid < m.GetPrimitiveCount())
	{
		uint index_offset = 3 * (m.GetPrimitiveOffset() + gtid);
		tris[gtid] = uint3(
			meshInstanceInfo.GetPrimitive_indices()[index_offset],
			meshInstanceInfo.GetPrimitive_indices()[index_offset + 1],
			meshInstanceInfo.GetPrimitive_indices()[index_offset + 2]);
		prims[gtid].slice = slot;
	}

	if (gtid < m.GetVertexCount())
	{
		uint vertexIndex = meshInfo.GetVertex_offset_local() + meshInstanceInfo.GetUnique_indices()[m.GetVertexOffset() + gtid];
		mesh_vertex_input v = meshInstanceInfo.GetVertexes()[vertexIndex];
		float4 wpos = mul(node_mat, float4(v.pos, 1));
		verts[gtid].pos = mul(page_cam.GetViewProj(), wpos);
		verts[gtid].wpos = wpos.xyz;
		verts[gtid].tc = v.tc;
	}
}
