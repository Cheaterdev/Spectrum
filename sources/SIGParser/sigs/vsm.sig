
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
	# Phase 5.7: the active range is a contiguous [active_min, active_max]
	# window over the fixed level_info[] ladder, recomputed every frame in
	# VSM.cpp's update_active_window() -- no more separate "regular ring
	# count" field. Finest-first: get_vsm_level walks active_min upward.
	int active_min;
	int active_max;
	int page_size;
	int pages_per_level;
	float4x4 light_view;
	# MaxLevels (VSM.ixx) storage slots -- one geometric ladder, no
	# regular/adaptive split (see VSMClipmap::page_world_size). Keep this in
	# step with VSM::MaxLevels.
	float4 level_info[26];
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

# Phase 5.14: batches the copy step across every dirty page at once (Z
# dimension = dirty page count this frame) instead of one dispatch per
# dirty page -- atlas/dst_mip0 are whole-array views (every physical slot),
# and dirty_slots.Load(z) picks which slice each Z-group actually writes.
[Bind = DefaultLayout::Instance0]
struct VSMCopyPageDepthBatch
{
	# atlas / dst_mip0 are NARROWED views (one mip, physical_page_count array
	# slices). Without [Barrier = ALL] each bind expands into one barrier per
	# slice; the whole resource is being rewritten by this step anyway, so one
	# whole-resource transition is both correct and far cheaper.
	[Barrier = ALL] Texture2DArray<float> atlas;
	[Barrier = ALL] RWTexture2DArray<float> dst_mip0;
	StructuredBuffer<uint> dirty_slots;
}

ComputePSO VSMCopyPageDepthBatch
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = vsm_copy_page_depth_batch;
}

# Phase 5.14: one dispatch per mip level, covering every dirty page at once
# (Z dimension = dirty page count), replacing the old one-dispatch-per-
# dirty-page-per-mip loop. src is a single SRV spanning the WHOLE mip
# chain (Load's 4th component selects src_mip directly -- HLSL can address
# any mip of a multi-mip SRV without a separate view per mip); only the
# write side needs a mip-specific view, since a UAV can only ever address
# one mip.
[Bind = DefaultLayout::Instance0]
struct VSMDownsampleHiZBatch
{
	# Both sides are narrowed to exactly one mip across physical_page_count
	# slices, and this runs once per mip per frame -- the single worst
	# subresource-expansion site in the frame. See VSMCopyPageDepthBatch.
	[Barrier = ALL] Texture2DArray<float> src;
	[Barrier = ALL] RWTexture2DArray<float> dst_mip;
	StructuredBuffer<uint> dirty_slots;
	uint src_mip;
}

ComputePSO VSMDownsampleHiZBatch
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = vsm_hiz_downsample_batch;
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
	# Pre-baked screen-space noise (see BlueNoise.sig) -- rotates the PCSS
	# Poisson disc per pixel (VSM_impl.hlsl's get_shadow_vsm) so the fixed
	# 16-tap pattern doesn't read as a rigid, repeating grid at wide radii.
	# A plain field on the already-Instance2-bound VSMLighting rather than
	# re-binding the whole BlueNoise struct (which wants Instance0, already
	# taken here by VSMConstants) -- same pattern VoxelGI/ReflectionDenoiser
	# already use for consuming this same baked texture.
	Texture2D<float2> blue_noise;
}

ComputePSO VSMApplyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_RESULT]
	compute = VSM;

	# Fixed single-tap 3x3 hardware-PCF (off) vs blocker-search + penumbra-
	# scaled PCF (on) -- see get_shadow_vsm in VSM_impl.hlsl.
	[rename = VSM_PENUMBRA]
	[CS, nullable]
	define VsmPenumbra;
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

# Phase 5.8: per-(level,mesh) indirect draw entry, replacing the CPU
# "for level { for mesh { dispatch_mesh() } }" loop with one exec_indirect
# call. Shaped exactly like meshrender.sig's CommandData (pointer fields to
# Bind-tagged CBV structs + a DispatchMeshArguments) -- page_batch_cb takes
# VSMPageBatch's place of CommandData's MaterialInfo*, carrying this entry's
# level/dirty_mask/skip_occlusion instead of per-level root-constant binds.
# No [shader_only] (unlike Dispatch*Arguments) -- CommandData is the
# precedent: this needs a C++-side Table:: struct too, since entries are
# built CPU-side in plan_frame(), not GPU-populated like CommandData is.
#
# Field order matters: D3D12 requires a command signature's root-parameter
# updates to be in strictly increasing {RootParameterIndex, offset} order.
# VSMPageBatch is DefaultLayout::Instance0, MeshInfo is Instance1,
# MeshInstanceInfo is Instance2 -- page_batch_cb must come first, or
# CreateCommandSignature fails with "Root parameter {slots, offset} must be
# increasing" (confirmed the hard way).
[IndirectCommand]
struct VSMDispatchCommandData
{
	VSMPageBatch* page_batch_cb;
	MeshInfo* mesh_cb;
	MeshInstanceInfo* meshinstance_cb;
	DispatchMeshArguments draw_commands;
}

# Phase 5.12: one entry per active+dirty LEVEL this frame (bounded by level
# count, not mesh count -- plan_frame() already decided this list, same as
# the VSMPageBatch CBs it already compiles per level). VSM_GatherDispatch
# tests every scene mesh's AABB against every entry's bounds and appends a
# VSMDispatchCommandData for each overlap, replacing VSM.cpp's old CPU
# "for level { for mesh { ... } }" loop.
struct VSMLevelDispatchInfo
{
	VSMPageBatch* page_batch_cb;
	float2 bounds_min;
	float2 bounds_max;
}

[Bind = DefaultLayout::Instance1]
struct VSMGatherDispatchData
{
	StructuredBuffer<VSMLevelDispatchInfo> levels;
	uint level_count;
	# Same rotation-only light-space view make_light_view_camera() builds
	# CPU-side (VSM.cpp) -- mesh AABBs are transformed into this space here
	# to compare against each level's bounds_min/bounds_max, computed in the
	# same space CPU-side.
	float4x4 light_view;
	AppendStructuredBuffer<VSMDispatchCommandData> dispatch_commands;
}

ComputePSO VSMGatherDispatch
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = vsm_gather_dispatch;
}

# GPU-driven replacement for VSM.cpp's old per-frame scene->iterate_meshes()
# walk (Phase 5.12) -- reuses the scene's own already-per-frame-built mesh
# list/count (Scene.cpp's compiledGather[MESH_TYPE::ALL], the same data the
# main camera's GatherMeshes pipeline already consumes every frame) and
# SceneData (already bound the same out-of-band way VSM_RenderPages binds
# it) rather than declaring either as FrameGraph resources here -- neither
# is owned or created by VSM.
[Compute]
PassNode VSM_GatherDispatch
{
	[Write] StructuredBuffer<VSMLevelDispatchInfo> VSM_LevelDispatchInfo;
	# AppendStructuredBuffer here (not just in VSMGatherDispatchData below) is
	# not supported by FrameGraph's PassNode codegen -- FrameGraph::Handlers
	# only implements the resource-tracking wrapper for plain
	# StructuredBuffer (confirmed the hard way: C2039, Handlers has no
	# AppendStructuredBuffer). The underlying HAL::StructuredBufferView<T>
	# already carries both .structuredBuffer and .appendStructuredBuffer
	# sub-views regardless of which one the FrameGraph field declares --
	# VSM_GatherDispatch's render() uses ->appendStructuredBuffer directly.
	[Write] StructuredBuffer<VSMDispatchCommandData> VSM_DispatchCommands;
}

# Single pass, not [Multiple=N]: VSM_GatherDispatch already decided exactly
# which (level,mesh) pairs need drawing and appended them; this pass just
# issues one exec_indirect() call over the resulting (GPU-counted) list.
PassNode VSM_RenderPages
{
	[Write] Texture VSM_Atlas;
	[Write] Texture VSM_PageTable;
	[Write] StructuredBuffer<Camera> VSM_PageCameras;
	# Still [Write] and still created here (not in VSM_HiZRebuild below):
	# the once-ever cold-start clear runs in this pass's render(), before
	# the draw reads it for occlusion, so data.VSM_PageHiZ.is_new() has to
	# be queryable on THIS pass's own handler -- matching the precedent
	# that is_new() is not valid on a handler that never create()'d/need()'d
	# the resource in that pass. VSM_HiZRebuild need()s the same resource
	# for the actual per-frame rebuild writes.
	[Write] Texture VSM_PageHiZ;
	StructuredBuffer<VSMDispatchCommandData> VSM_DispatchCommands;
}

# Phase 5.17: Hi-Z pyramid rebuild, split into its own async-compute pass.
# Nothing this frame reads VSM_PageHiZ after VSM_RenderPages' own draw --
# the mesh shader's occlusion test only ever consults whatever pyramid
# content was left by the LAST time a page was rebuilt (a page's own
# last-rendered depth is always at least one frame stale by design; see
# the invalidation tracker's job of covering for that). So this pass has
# nothing forcing it to finish before the rest of the frame's direct-queue
# work: it only needs to run after VSM_RenderPages' draw (reads VSM_Atlas,
# which that pass just wrote) and before NEXT frame's VSM_RenderPages draw
# (which reads VSM_PageHiZ). [Compute] puts it on the async compute queue
# instead of serializing it into the direct queue's own critical path.
[Compute]
PassNode VSM_HiZRebuild
{
	Texture VSM_Atlas;
	[Write] Texture VSM_PageHiZ;
	# Phase 5.14: this frame's flat list of dirty physical slots, CPU-built
	# and uploaded once, consumed by the batched Hi-Z copy/downsample
	# dispatches (VSMCopyPageDepthBatch/VSMDownsampleHiZBatch). Moved here
	# from VSM_RenderPages along with the rest of the per-frame rebuild.
	[Write] StructuredBuffer<uint> VSM_DirtySlots;
}

[Compute]
PassNode VSM_Combine
{
	GBuffer gbuffer;
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	Texture BlueNoise;
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
