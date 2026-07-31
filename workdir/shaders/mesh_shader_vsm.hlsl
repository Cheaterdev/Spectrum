// VSM-only mesh shader: renders one clipmap level's worth of pages (up to
// pages_per_level^2, routed via SV_ViewportArrayIndex) in a single
// DispatchMesh call per node, instead of one CPU-issued draw per page.
//
// Deliberately independent from mesh_shader.hlsl (used by GBuffer/PSSM) --
// no shared file is touched, so this cannot regress either of those.
//
// Per-meshlet culling (Phase 1b): each (meshlet, page) thread group tests
// the meshlet's bounding sphere + normal cone against that page's own
// camera before doing any vertex work, mirroring mesh_shader.hlsl's
// IsVisible() (already proven correct against a light camera, not just the
// main view -- PSSM's cascades use the same function against their own
// light_cam). Still no amplification-shader-based compaction: an
// invisible (meshlet, page) pair skips its vertex/index work but the
// thread group itself still launches -- true compaction (skipping the
// dispatch itself) is a further optimization, not done here.

#include "Common.hlsl"

#include "autogen/MeshInfo.h"
#include "autogen/SceneData.h"
#include "autogen/MeshInstanceInfo.h"
#include "autogen/VSMPageBatch.h"
#include "autogen/VSMPageTableData.h"

static const MeshInfo meshInfo = GetMeshInfo();
static const SceneData sceneData = GetSceneData();
static const MeshInstanceInfo meshInstanceInfo = GetMeshInstanceInfo();

struct vsm_vertex_output
{
	float4 pos : SV_POSITION;
};

struct vsm_prim_attrs
{
	uint viewport : SV_ViewportArrayIndex;
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

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void VS(
	uint gtid : SV_GroupThreadID,
	uint3 gid3 : SV_GroupID,   // gid3.x = meshlet index, gid3.y = page index within this level's batch
	out indices uint3 tris[64],
	out vertices vsm_vertex_output verts[128],
	out primitives vsm_prim_attrs prims[64]
)
{
	uint meshletIndex = gid3.x;
	uint pageLocal = gid3.y;

	// Clamp to something always safe to read -- avoids a second conditional
	// path feeding into anything upstream of SetMeshOutputCounts.
	bool in_bounds = meshletIndex < meshInfo.GetMeshlet_count();
	uint safe_index = in_bounds ? meshletIndex : 0;

	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[GetVSMPageBatch().GetPage_base_slot() + pageLocal];

	node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
	matrix node_mat = node.GetNode_global_matrix();

	MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + safe_index];
	bool visible = in_bounds && vsm_is_visible(cull_data, node_mat, page_cam);

	Meshlet m = meshInstanceInfo.GetMeshlets()[meshInfo.GetMeshlet_offset_local() + safe_index];

	// SetMeshOutputCounts must be called exactly once, with no conditional
	// call sites at all -- DXC rejects even textually-single, ternary-fed
	// calls here as "cannot be called multiple times" once vsm_is_visible's
	// unrolled loop gets inlined, and a thread group that never calls it
	// leaves its output state undefined (rasterizes stale data left over
	// from whatever group last used that physical output slot -- this is
	// what caused the earlier "exploded triangles"). So: always declare the
	// meshlet's real counts unconditionally, and cull by writing degenerate
	// (zero-area) triangles for invisible/out-of-bounds groups instead of
	// shrinking the declared counts.
	SetMeshOutputCounts(m.GetVertexCount(), m.GetPrimitiveCount());

	if (gtid < m.GetPrimitiveCount())
	{
		if (visible)
		{
			uint index_offset = 3 * (m.GetPrimitiveOffset() + gtid);
			tris[gtid] = uint3(
				meshInstanceInfo.GetPrimitive_indices()[index_offset],
				meshInstanceInfo.GetPrimitive_indices()[index_offset + 1],
				meshInstanceInfo.GetPrimitive_indices()[index_offset + 2]);
		}
		else
		{
			tris[gtid] = uint3(0, 0, 0); // degenerate, zero area, never rasterizes
		}
		prims[gtid].viewport = pageLocal;
	}

	if (gtid < m.GetVertexCount())
	{
		if (visible)
		{
			uint vertexIndex = meshInfo.GetVertex_offset_local() + meshInstanceInfo.GetUnique_indices()[m.GetVertexOffset() + gtid];
			float4 wpos = mul(node_mat, float4(meshInstanceInfo.GetVertexes()[vertexIndex].pos, 1));
			verts[gtid].pos = mul(page_cam.GetViewProj(), wpos);
		}
		else
		{
			verts[gtid].pos = float4(0, 0, 0, 1);
		}
	}
}
