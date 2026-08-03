// VSM-only mesh shader: renders one clipmap level's worth of pages (up to
// pages_per_level^2, routed via SV_ViewportArrayIndex) in a single
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

// pages_per_level is pinned at 4x4=16 (matches VSM.cpp / VSMClipmap -- the
// D3D12 16-viewport-per-draw cap the single-dispatch-per-level design
// depends on). If that ever changes, this must change with it.
#define VSM_PAGES_PER_LEVEL 16

struct vsm_vertex_output
{
	float4 pos : SV_POSITION;
};

struct vsm_prim_attrs
{
	uint viewport : SV_ViewportArrayIndex;
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

// Phase 3: occlusion test against this page's OWN Hi-Z pyramid, built from
// the LAST time this page rendered (see VSM.cpp's rebuild loop) -- not this
// frame's not-yet-drawn geometry. Single-phase by design: VSM pages are
// cached indefinitely (Phase 2), so there's no same-frame retest like the
// main camera's two-phase occlusion; a moving occluder is expected to
// invalidate both its own page and whatever it was hiding via the same
// AABB-to-page mapping in VSM::attach_scene.
//
// Sphere-only (no per-meshlet AABB exists anywhere in this codebase). VSM's
// page cameras are orthographic (VSM.cpp's set_projection_params(l,r,t,b,
// znear,zfar)), so unlike a perspective test, screen-space footprint is
// depth-independent -- no perspective divide needed for the radius, and the
// "nearest point toward the light" is exact by pushing the center back along
// the page camera's forward axis, not an approximation of a curved
// projection.
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
	// Same NDC->page-UV convention as get_shadow_vsm() in VSM_impl.hlsl.
	float2 page_uv = center_clip.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	// Sphere center outside this page: let the frustum test's own
	// conservative sphere-vs-plane result stand rather than guessing.
	if (any(page_uv < 0) || any(page_uv > 1))
		return false;

	Texture2DArray<float> pyramid = GetVSMPageHiZ().GetPage_hiz();
	uint pw, ph, elems, numLevels;
	pyramid.GetDimensions(0, pw, ph, elems, numLevels);

	// Screen-space radius via NDC distance to an offset point, reusing the
	// same mul(viewProj, pos) pattern as everywhere else in this file --
	// deliberately NOT indexing page_cam.GetProj() directly (its row/column
	// layout in this codegen was never verified, and a wrong assumption
	// there would silently pick too-fine a mip, sampling a single texel
	// instead of the properly conservative footprint -- exactly the kind of
	// bug that shows up as sporadic, patchy false occlusion rather than
	// something consistently broken). Orthographic, so the measured radius
	// is the same regardless of which perpendicular direction is used.
	float3 world_up = (abs(forward.y) > 0.99) ? float3(1, 0, 0) : float3(0, 1, 0);
	float3 right = normalize(cross(world_up, forward));
	float4 edge_clip = mul(page_cam.GetViewProj(), float4(center.xyz + right * radius, 1));
	edge_clip.xyz /= edge_clip.w;
	float radius_ndc = length(edge_clip.xy - center_clip.xy);
	float radius_texels = radius_ndc * 0.5 * (float)ph;
	uint mip = (uint)clamp(ceil(log2(max(radius_texels * 2.0, 1.0))), 0.0, (float)(numLevels - 1));

	float sampled = pyramid.SampleLevel(pointClampSampler, float3(page_uv, (float)slot), mip);

	// reversed-Z: occluded if even the sphere's nearest point is farther
	// (numerically smaller) than the farthest depth already recorded at that
	// texel/mip -- see downsample_depth.hlsl's min-reduction.
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

	int slot = GetVSMPageBatch().GetPage_base_slot() + pageLocal;
	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[slot];
	node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
	matrix node_mat = node.GetNode_global_matrix();
	MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + meshletIndex];

	// Phase 2 per-page caching: a page outside this frame's dirty mask keeps
	// its existing content even if this meshlet would otherwise be visible
	// there -- CPU only clears the pages actually being re-rendered, so
	// skipping the geometry here is what makes that clear valid.
	bool page_dirty = (GetVSMPageBatch().GetDirty_mask() >> pageLocal) & 1;
	bool visible = valid && page_dirty && vsm_is_visible(cull_data, node_mat, page_cam);

	// Phase 3: occlusion is an additional filter on top of frustum+dirty,
	// not a replacement -- an occluded pair on a dirty page must still not
	// launch mesh-shader work. Skipped on a recenter/light-move redraw:
	// this slot's pyramid still holds an unrelated old world region (or the
	// same region under a different light angle), so it can't validly
	// occlude this frame's geometry -- see VSMPageBatch::skip_occlusion.
	bool skip_occlusion = GetVSMPageBatch().GetSkip_occlusion() != 0;
	if (visible && !skip_occlusion)
		visible = !vsm_is_occluded(cull_data, node_mat, page_cam, slot);

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

	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[GetVSMPageBatch().GetPage_base_slot() + pageLocal];
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
		prims[gtid].viewport = pageLocal;
	}

	if (gtid < m.GetVertexCount())
	{
		uint vertexIndex = meshInfo.GetVertex_offset_local() + meshInstanceInfo.GetUnique_indices()[m.GetVertexOffset() + gtid];
		float4 wpos = mul(node_mat, float4(meshInstanceInfo.GetVertexes()[vertexIndex].pos, 1));
		verts[gtid].pos = mul(page_cam.GetViewProj(), wpos);
	}
}
