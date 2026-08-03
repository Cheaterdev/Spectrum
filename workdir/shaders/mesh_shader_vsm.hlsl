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

	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[GetVSMPageBatch().GetPage_base_slot() + pageLocal];
	node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
	matrix node_mat = node.GetNode_global_matrix();
	MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + meshletIndex];

	// Phase 2 per-page caching: a page outside this frame's dirty mask keeps
	// its existing content even if this meshlet would otherwise be visible
	// there -- CPU only clears the pages actually being re-rendered, so
	// skipping the geometry here is what makes that clear valid.
	bool page_dirty = (GetVSMPageBatch().GetDirty_mask() >> pageLocal) & 1;
	bool visible = valid && page_dirty && vsm_is_visible(cull_data, node_mat, page_cam);

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
