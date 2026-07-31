// VSM-only mesh shader: renders one clipmap level's worth of pages (up to
// pages_per_level^2, routed via SV_ViewportArrayIndex) in a single
// DispatchMesh call per node, instead of one CPU-issued draw per page.
//
// Deliberately independent from mesh_shader.hlsl (used by GBuffer/PSSM) --
// no shared file is touched, so this cannot regress either of those. No
// amplification shader / meshlet culling yet (Phase 1b/3 territory): the
// CPU dispatches ceil(meshlet_count/32) x page_count x 1 threadgroups
// directly, one meshlet per X, one page per Y.
//
// KNOWN ISSUE (under active debugging): only viewport index 0 of the bound
// array ever receives geometry -- forcing prims[gtid].viewport to any fixed
// non-zero value makes everything vanish, while leaving it at pageLocal
// only ever shows content in whichever pass's own local index 0 happens to
// land (confirmed via the FrameGraph debugger's VSM_Atlas view). Root cause
// not yet found -- the C++ call chain (GraphicsContext::set_viewports ->
// DelayedCommandList::set_viewports -> CommandList::set_viewports ->
// RSSetViewports) and the Viewport->D3D12_VIEWPORT field mapping both look
// correct on inspection.

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
	if (meshletIndex >= meshInfo.GetMeshlet_count())
		return;

	Meshlet m = meshInstanceInfo.GetMeshlets()[meshInfo.GetMeshlet_offset_local() + meshletIndex];
	SetMeshOutputCounts(m.GetVertexCount(), m.GetPrimitiveCount());

	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[GetVSMPageBatch().GetPage_base_slot() + pageLocal];

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
		node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
		matrix node_mat = node.GetNode_global_matrix();

		float4 wpos = mul(node_mat, float4(meshInstanceInfo.GetVertexes()[vertexIndex].pos, 1));
		verts[gtid].pos = mul(page_cam.GetViewProj(), wpos);
	}
}
