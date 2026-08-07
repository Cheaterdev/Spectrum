
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
	float4x4 light_view;
	# MaxLevels (VSM.ixx) storage slots: regular_level_count (6) rings plus
	# AdaptiveTierCount (3) adaptive tiers -- keep this in step with both.
	float4 level_info[9];
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

# Copies one page's rendered slice of VSM_Atlas into VSMPageHiZ's mip 0.
# Both are per-page slices of the same size, so it is a straight 1:1 copy.
[Bind = DefaultLayout::Instance0]
struct VSMCopyPageDepth
{
	Texture2D<float> atlas;
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
	# Which clipmap level this draw covers. Physical slots are no longer a
	# fixed function of (level, local page) -- the AS/VS look the slot up
	# per local page via VSMPageTableData's page_table (real allocator,
	# see VSMPageTable.ixx).
	int level;
	# Bit i = local page i (0..pages_per_level^2-1, currently 16) is dirty
	# this frame. Phase 2 per-page invalidation: a page outside this mask
	# is skipped by the AS even if visible, keeping its cached content.
	int dirty_mask;
	# Bit i = local page i has no valid Hi-Z history this frame (freshly
	# (re)allocated, or the light moved) -- the AS must skip occlusion for
	# it. Per-page, not per-level: a page whose slot/content survived a
	# recenter keeps its history even when siblings don't.
	int skip_occlusion;
}

[Bind = DefaultLayout::Instance2]
struct VSMLighting
{
	GBuffer gbuffer;
	Texture2DArray<float> vsm_atlas;
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

# One slot per storage level -- both regular rings and Phase 5.6 adaptive
# tiers, since an adaptive tier is a normal level with a fixed size and an
# on/off flag, not a separate mechanism (see VSM.ixx's AdaptiveTierCount).
# Must match VSM::MaxLevels / VSMConstants::level_info[]'s size.
[Multiple = 9]
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

# Phase 5.6: single-value feedback for the adaptive-tier hysteresis in
# VSM.cpp's plan_frame() -- MIN world-space texel size (screen pixel
# footprint projected to world space) over the central screen region,
# reduced via InterlockedMin on the bit pattern of a positive float (valid
# since IEEE754 preserves ordering under uint reinterpretation for positive
# values). Static like VSM_PageCameras: cleared then written each frame,
# read back on the copy queue afterward (VisibilityBuffer.cpp's
# process_tile_readback is the precedent for the copy.read<T> callback
# pattern this follows) -- letting the FrameGraph track this resource
# normally handles the cross-queue synchronization automatically. Result
# is inherently last frame's, same latency class as any GPU feedback loop.
#
# [Required]: nothing ever reads VSM_DepthAnalysisResult through the normal
# graph dependency system -- its only consumer is the out-of-band
# copy.read<T> callback in VSM.cpp's render(), which the FrameGraph has no
# visibility into. Without [Required], a pass whose output nothing else
# declares a read dependency on gets silently discarded before actual GPU
# submission: no crash, no validation error, no shader error -- it just
# never runs on the GPU, so the readback callback registered inside it never
# fires either. (Confirmed the hard way: removed manually-redundant resource
# transitions first as the suspected cause, no change; this was the actual
# one.)
[Bind = DefaultLayout::Instance0]
struct VSMDepthAnalysis
{
	GBuffer gbuffer;
	RWStructuredBuffer<uint> result;
}

ComputePSO VSMDepthAnalysis
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = vsm_depth_analysis;
}

[Compute]
[Required]
PassNode VSM_DepthAnalysis
{
	GBuffer gbuffer;
	[Write] StructuredBuffer<uint> VSM_DepthAnalysisResult;
}
