
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

# Phase 3: per-physical-atlas-slot Hi-Z pyramid, one array slice per slot
# (not packed into VSM_Atlas -- mip downsampling would bleed across tiles).
# Instance4: Instance0/1/2/3 are all already taken in VSMDepthDraw (see
# VSMPageTableData above).
[Bind = DefaultLayout::Instance4]
struct VSMPageHiZ
{
	Texture2DArray<float> page_hiz;
}

# Copies one page's rendered region of VSM_Atlas into VSMPageHiZ's mip 0.
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
	# Nonzero when dirty_mask came from a recenter/light-move, not a scene
	# change -- the slot's Hi-Z pyramid is then stale (different world
	# region or light angle), so the AS must skip occlusion this draw.
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
