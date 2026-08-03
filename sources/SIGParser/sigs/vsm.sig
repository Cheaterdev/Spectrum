
# Virtual Shadow Map (VSM), directional-light clipmap, Phase 1a.
#
# Architecture: a small CPU-authoritative page table (VSMPageTable.ixx) maps
# (clip level, page coord) to a slot in a single physical atlas texture.
# VSM_RenderPage is one pass per clipmap LEVEL (not per page): a single scene
# traversal, mesh-shader-only dispatch (no amplification/culling stage yet),
# with SV_ViewportArrayIndex routing each meshlet's output to all
# pages_per_level^2 pages of that level in one DispatchMesh call (Y dimension
# = page index -- see mesh_shader_vsm.hlsl). Sampling happens through
# VSM_Combine, which is fully independent from PSSM_Combine. PSSM is left
# untouched.

[Bind = DefaultLayout::Instance0]
struct VSMConstants
{
	int level_count;
	int page_size;
	int pages_per_level;
	int atlas_pages_per_side;
	float4x4 light_view;
	float4 level_info[8];
}

# Instance3, not Instance1: mesh_shader_vsm.hlsl needs this alongside MeshInfo
# (a fixed non-Instance slot), and MeshInfo happens to land on the same raw
# slot number as Instance1 -- Instance3 avoids the collision.
[Bind = DefaultLayout::Instance3]
struct VSMPageTableData
{
	Texture2DArray<uint> page_table;
	StructuredBuffer<Camera> page_cameras;
}

# Phase 3: per-physical-atlas-slot Hi-Z pyramid (one array slice per slot,
# each its own mip chain -- NOT packed into VSM_Atlas itself, since mip
# downsampling across shared-atlas tile edges would bleed neighboring pages'
# depth into each other). Array slice = physical atlas slot, same indexing
# VSM_Atlas's viewport routing already uses (page_base_slot + local page).
#
# Instance4, not Instance3: mesh_shader_vsm.hlsl's AS/VS need this alongside
# VSMPageTableData in the SAME PSO (VSMDepthDraw), and Instance0/2/3 are
# already taken there (VSMPageBatch/MeshInstanceInfo/VSMPageTableData) --
# Instance1 is also unsafe (aliases MeshInfo's fixed slot, see
# VSMPageTableData's comment above).
[Bind = DefaultLayout::Instance4]
struct VSMPageHiZ
{
	Texture2DArray<float> page_hiz;
}

# Copies one page's just-rendered region of VSM_Atlas into VSMPageHiZ's mip 0
# for that slot -- a small dedicated shader rather than a generic sub-rect
# SRV view, since VSM_Atlas (one monolithic depth texture) and VSMPageHiZ
# (a Texture2DArray) are different resource shapes.
[Bind = DefaultLayout::Instance0]
struct VSMCopyPageDepth
{
	Texture2D<float> atlas;
	int2 atlas_origin;
	RWTexture2D<float> dst_mip0;
}

ComputePSO VSMCopyPageDepth
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = vsm_copy_page_depth;
}

[Bind = DefaultLayout::Instance0]
struct VSMPageBatch
{
	int page_base_slot;
	# Bit i = local page i (0..pages_per_level^2-1, currently 16) is dirty
	# this frame. Phase 2 per-page invalidation: a page outside this mask
	# is skipped by the AS even if visible, keeping its cached content.
	int dirty_mask;
	# Phase 3: nonzero when this level's dirty_mask came from a full
	# recenter/light-move invalidation rather than a per-object scene
	# change. On a recenter, the SAME physical atlas slot now represents a
	# DIFFERENT world region (or, on light-move, the same region under a
	# different light angle) -- its Hi-Z pyramid still holds whatever was
	# there before, which has nothing to do with this frame's geometry.
	# The AS must skip the occlusion test in that case (frustum-only),
	# since testing against that stale pyramid data would falsely occlude
	# real geometry (see the "meshlets not rasterized" regression this
	# fixed).
	int skip_occlusion;
}

[Bind = DefaultLayout::Instance2]
struct VSMLighting
{
	GBuffer gbuffer;
	Texture2D<float> vsm_atlas;
	Texture2DArray<uint> page_table;
	StructuredBuffer<Camera> page_cameras;
	RWTexture2D<float4> result;
}

ComputePSO VSMApplyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_RESULT]
	compute = VSM;
}

# Amplification-shader-driven compaction (Phase 1b): CPU dispatches AS
# threadgroups covering meshlet_count*16 (meshlet,page) pairs; the AS culls
# each pair against that page's camera and compacts survivors into a
# payload, so an invisible pair never launches a mesh-shader threadgroup at
# all (earlier version culled inside the MS, which still launched every
# group and wrote degenerate triangles for culled ones). Mesh shader routes
# each output primitive to a viewport via SV_ViewportArrayIndex.
GraphicsPSO VSMDepthDraw
{
	root = DefaultLayout;

	[Erase]
	pixel = null;

	[EntryPoint = VS]
	mesh = mesh_shader_vsm;

	[EntryPoint = AS]
	amplification = mesh_shader_vsm;

	ds = D32_FLOAT;
	cull = Front;
}

# One slot per clipmap level (not per page anymore -- see mesh_shader_vsm.hlsl).
# MaxCount is a generous Phase-1a budget; level_count itself is set at
# runtime in VSM.cpp and must stay <= this.
[Multiple = 8]
PassNode VSM_RenderPage
{
	[Write] Texture VSM_Atlas;
	[Write] Texture VSM_PageTable;
	[Write] StructuredBuffer<Camera> VSM_PageCameras;
	[Write] Texture VSM_PageHiZ;
}

[Compute]
PassNode VSM_Combine
{
	GBuffer gbuffer;
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	[Write] Texture ResultTexture;
}
