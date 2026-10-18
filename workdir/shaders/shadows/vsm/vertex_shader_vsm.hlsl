// VSM page rendering for GPUs without mesh shaders (see mesh_shader_vsm.hlsl, which does the same per meshlet).
// One direct indexed draw per (clipmap level, mesh part), instanced once per page of the level: the instance is the
// page, its physical slot (the atlas array slice it renders into) comes from the page table, and the level's
// VSMPageBatch is bound once for the draw (draw_id == DRAW_ID_NONE, so vsm_page_batch() reads pass_VSMPageBatch).
// There is no culling stage: a page that is not redrawn this frame, or has no slot, collapses to a degenerate
// triangle and rasterizes nothing.
#define VERTEX_FRONT_END
#include "mesh_shader_vsm.hlsl"

struct vsm_vertex_front_output
{
	float4 pos : SV_POSITION;
	float3 wpos : POSITION;
	float2 tc : TEXCOORD;
	uint slice : SV_RenderTargetArrayIndex;
#ifdef __spirv__
	nointerpolation uint draw_id_out : DRAW_ID;
#endif
};

vsm_vertex_front_output VSVertex(uint vertex_id : SV_VertexID, uint page_local : SV_InstanceID)
{
	bind_draw(read_draw_id());

	vsm_vertex_front_output o;
	o.wpos = 0;
	o.tc = 0;
	o.slice = 0;
#ifdef __spirv__
	o.draw_id_out = draw_id;
#endif

	const bool page_dirty = (GetVSMPageBatch().GetDirty_mask() >> page_local) & 1;
	const uint slot = vsm_slot_of(page_local);
	if (!page_dirty || slot == VSM_INVALID_SLOT)
	{
		o.pos = 0;   // every vertex of the triangle at one point: nothing to rasterize
		return o;
	}

	node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
	mesh_vertex_input v = meshInstanceInfo.GetVertexes()[meshInfo.GetVertex_offset_local() + vertex_id];
	float4 wpos = mul(node.GetNode_global_matrix(), float4(v.pos, 1));

	Camera page_cam = GetVSMPageTableData().GetPage_cameras()[slot];
	o.pos = mul(page_cam.GetViewProj(), wpos);
	o.wpos = wpos.xyz;
	o.tc = v.tc;
	o.slice = slot;
	return o;
}
