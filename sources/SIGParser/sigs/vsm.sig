
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
	# 0 = single blur pass (uses the RTX-verified distance when the
	# verification ray hit something, VSM's own estimate otherwise -- cheaper,
	# one 16-tap blur per pixel). 1 = blur BOTH distances and take min() of
	# the two resulting shadow values (pricier -- an extra 16-tap blur
	# whenever the ray hits -- but avoids the bright spots a single blended
	# estimate produced between overlapping penumbras). Only read when
	# VsmRtxVerify is enabled.
	int rtx_dual_blur;
	# 0 = every pixel runs its own full 16-tap blocker search (original).
	# 1 = split the 16 taps 4-per-thread across each 2x2 pixel quad, merged
	# via QuadReadAcrossX/Y/Diagonal -- ~4x fewer atlas samples per pixel for
	# the search, same total 16-tap coverage. New/unverified, hence a
	# runtime A/B switch rather than replacing the original outright.
	int quad_blocker_search;
	# Debug view: when nonzero, VSM_Combine ignores get_shadow_vsm entirely
	# for real geometry pixels and instead displays RTXShadow's own
	# (denoised) full-RT shadow mask directly as grayscale -- a reference
	# to compare VSM's quality/performance against. See VSMLighting's
	# rtx_shadow_mask field.
	int debug_rtx_reference;
	# Runtime A/B switch (Phase 5.18 Part A) for the min/max Hi-Z
	# classification vsm_search_blocker does against VSMPageHiZ before
	# running its 16-tap search -- nonzero = skip the search entirely for
	# pixels the pyramid alone can already answer confidently (see
	# vsm_search_blocker's own comment). Only read by VSM_BlockerSearch's
	# own dispatch; VSM_Combine never consults it.
	int hiz_blocker_classify;
	# Debug view: when nonzero, get_shadow_vsm (VSM_Combine's resolve step)
	# short-circuits into a flat color wherever vsm_search_blocker's Hi-Z
	# classification fired confidently instead of shading normally --
	# green for confident_lit, blue for confident_dark -- so the
	# classification's real firing pattern is visible directly, not just
	# its downstream effect on the shadow. Ambiguous/real-search pixels
	# still shade normally, for context. Only read by VSM_Combine's own
	# resolve dispatch; VSM_BlockerSearch never consults it.
	int debug_hiz_classify;
	# Debug view: when nonzero, get_shadow_vsm colors every pixel by which
	# clipmap level and which page WITHIN that level it resolved to --
	# one flat color per level (palette[level % 8], same palette the older
	# compile-time VSM_DEBUG_HEATMAP define already used), darkened on a
	# checkerboard by (page_x + page_y) parity so page SEAMS show up as a
	# visible brightness step, not just level boundaries. Built to make it
	# easy to tell whether a visual artifact (e.g. a thin line inside an
	# otherwise-confident Hi-Z classification region) lines up with an
	# actual page/level boundary or not. Only read by VSM_Combine's own
	# resolve dispatch.
	int debug_page_grid;
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
#
# Phase 5.18 Part A: two channels, not one. .x = MIN reduction (farthest-
# from-light in footprint -- the original, unchanged occlusion-culling
# semantics: mesh_shader_vsm.hlsl's vsm_is_occluded still only ever reads
# this channel). .y = MAX reduction (closest-to-light in footprint, new) --
# lets VSM_BlockerSearch's own classification (VSM_impl.hlsl's
# vsm_search_blocker) tell "definitely nothing can block here" (.y still
# farther from light than the receiver) and "definitely everything blocks
# here" (.x still closer to light than the receiver) apart with one fetch,
# instead of only ever answering the first question.
[Bind = DefaultLayout::Instance4]
struct VSMPageHiZ
{
	Texture2DArray<float2> page_hiz;
}

# Copies one page's rendered slice of VSM_Atlas into VSMPageHiZ's mip 0.
# Both are per-page slices of the same size, so it is a straight 1:1 copy.
# NOTE: unused legacy single-page (non-batched) form, superseded by
# VSMCopyPageDepthBatch below (nothing calls PSOS::VSMCopyPageDepth any
# more) -- left as-is, not touched by the Phase 5.18 float2 change.
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
	# atlas stays single-channel (it's VSM_Atlas's real rendered depth);
	# dst_mip0 is VSMPageHiZ's mip 0, now float2 -- see VSMPageHiZ's own
	# comment. The copy shader writes float2(d, d): both channels start
	# equal at mip 0, min/max only diverge once downsampling reduces >1 texel.
	[Barrier = ALL] Texture2DArray<float> atlas;
	[Barrier = ALL] RWTexture2DArray<float2> dst_mip0;
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
	# float2: .x = running MIN (farthest), .y = running MAX (closest) -- see
	# VSMPageHiZ's own comment.
	[Barrier = ALL] Texture2DArray<float2> src;
	[Barrier = ALL] RWTexture2DArray<float2> dst_mip;
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
	# Reference-comparison debug view: RTXShadow's own (denoised) full-RT
	# shadow mask, same resource PSSM_Combine reads as an alternative to its
	# own cascade shadow maps. RTXShadow runs unconditionally every frame on
	# RTX-capable hardware regardless of which of PSSM/VSM is the active
	# shadow system, so this is available for VSM to sample too -- see
	# VSM.cpp's m_combine_setup for the builder.exists() guard (RTXShadow's
	# own setup() can return false on non-RTX hardware, in which case this
	# resource never gets created that frame).
	Texture2D<float> rtx_shadow_mask;
	# Blocker-search extraction: written by the new VSM_BlockerSearch pass
	# (one full-screen dispatch, same size as VSM_Combine's), read back here
	# by VSM_Combine's resolve step -- the wide, many-tap search no longer
	# runs inline inside the same dispatch as the final PCF blur/shading.
	# uint4, not float4: x = asuint(world_delta), or asuint(-1.0) as the
	# sentinel for "no blocker found" (world_delta is otherwise always >=0
	# by construction); y/z = asuint(best_tc.x)/asuint(best_tc.y); w =
	# best_slot directly (already a uint). Bit-reinterpreted rather than
	# stored as native floats so best_slot doesn't lose precision the way
	# it would packed into a half-float channel. RWTexture2D (not a plain
	# Texture2D SRV) because VSM_BlockerSearch's own shader writes it --
	# VSM_Combine only ever reads it, via the same field/binding.
	RWTexture2D<uint4> blocker_result;
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

	# Only meaningful together with VsmPenumbra (VSM.cpp only ever enables
	# this when penumbra is also on): once VSM's blocker search finds a
	# blocker, fires one RayQuery toward the sun to verify/correct its
	# distance against the real BVH -- shadow maps only record the front-
	# most surface per texel, so a closer blocker can exist without ever
	# being rasterized where the search looked. Needs RTX hardware; gated
	# at runtime in VSM.cpp, not just by this define, since VSM must keep
	# working correctly without it.
	[rename = VSM_RTX_VERIFY]
	[CS, nullable]
	define VsmRtxVerify;
}

# Blocker-search extraction (see VSM_BlockerSearch's own PassNode comment):
# no VsmPenumbra/VsmRtxVerify defines needed here -- VSM.cpp only ever runs
# this pass at all when penumbra mode is on (there's no blocker search
# without it), and RTX ray-firing happens entirely in the resolve step
# (VSMApplyCompute), not here.
ComputePSO VSMBlockerSearchCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_BLOCKER_SEARCH]
	compute = VSM_BlockerSearch;
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
	# Back to cull=Front (render only back faces -- avoids self-shadow acne
	# "for free" by using the far side of closed geometry as the recorded
	# blocker depth). Was briefly cull=None to fix single-sided/thin
	# geometry (leaves, cards, thin walls) casting no shadow at all with no
	# back face to rasterize -- but that traded a real, if narrow, bug for
	# a much messier one: cull=None reintroduces self-shadow acne on ALL
	# front-facing geometry, and compensating for it (normal-offset bias,
	# self-shadow dead zones, etc.) chased artifacts (gray penumbras, page-
	# edge flicker) for longer than the original single-sided-geometry gap
	# was worth. Accepting the narrower, well-understood limitation again.
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
# Phase 5.18 Part A: no longer true that nothing reads VSM_PageHiZ later
# this same frame -- VSM_BlockerSearch's classification step (VSM_impl.hlsl's
# vsm_search_blocker) now samples it too, so this pass has to actually
# finish before VSM_BlockerSearch runs, not just before next frame's
# VSM_RenderPages draw. test.sig's pipeline listing moves this immediately
# before VSM_BlockerSearch (both [Async2], same physical queue) so that
# ordering falls out of ordinary same-queue in-order execution rather than
# needing a new cross-queue fence. [Compute] still keeps it off the direct
# queue's own critical path -- it only needs to run after VSM_RenderPages'
# draw (reads VSM_Atlas, which that pass just wrote) and before
# VSM_BlockerSearch (and, as before, before NEXT frame's VSM_RenderPages
# draw).
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

# Blocker-search extraction: one full-screen dispatch (same size as
# VSM_Combine's) that runs ONLY the wide, many-tap PCSS blocker search
# (VSM_impl.hlsl's vsm_search_blocker), writing its result to
# VSM_BlockerResult for VSM_Combine's resolve step to read back -- no
# longer inline inside the same dispatch as the final PCF blur/shading.
#
# Phase 5.18 Part A: also reads VSM_PageHiZ now -- vsm_search_blocker does
# a cheap min/max classification against the receiver's own page pyramid
# before running the expensive 16-tap search, skipping the search entirely
# for pixels the pyramid alone can already answer confidently (see
# VSMPageHiZ's own comment for the two channels' meaning). This is why
# VSM_HiZRebuild moved to run immediately before this pass in test.sig's
# listing -- the pyramid this reads must be this frame's freshly-rebuilt
# one, not last frame's.
[Compute]
PassNode VSM_BlockerSearch
{
	GBuffer gbuffer;
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	Texture VSM_PageHiZ;
	Texture BlueNoise;
	[Write] Texture VSM_BlockerResult;
}

[Compute]
PassNode VSM_Combine
{
	GBuffer gbuffer;
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	Texture BlueNoise;
	# Same resource RTXShadow writes / PSSM_Combine reads -- see
	# VSMLighting's rtx_shadow_mask field for the full rationale. Not
	# [Write]: this pass only ever reads it, for the debug-view comparison
	# toggle (VSM.ixx's use_vsm_debug_rtx_reference).
	Texture ShadowMask;
	# Written by VSM_BlockerSearch above -- see VSMLighting's own
	# blocker_result field for the packed uint4 layout. Not [Write]: this
	# pass only ever reads it.
	Texture VSM_BlockerResult;
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
