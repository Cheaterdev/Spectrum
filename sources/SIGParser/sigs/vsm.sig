
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

# One mutually-exclusive debug-view selector, replacing three separate
# int flags (debug_page_grid/debug_rtx_reference/debug_hiz_classify) that
# were always meant to be single-select in the first place -- every
# consumer (VSM.hlsl's combine_result, VSM.cpp's m_debugoverlay_render)
# already picked one via a priority-ordered if/else chain (page grid, then
# rtx reference, then hiz classify), never actually reading more than one
# at a time. Shared C++/HLSL type (SIG enum codegen) instead of a hand-kept
# int convention, so VSM.ixx's Variable<VSMDebugView> and every shader
# check are the same values by construction, not by keeping two separately-
# maintained mappings in sync.
enum VSMDebugView
{
	None;
	PageGrid;
	RtxReference;
	HizClassify;
}

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
	# 0 = every pixel runs its own full 16-tap blocker search (original).
	# 1 = split the 16 taps 4-per-thread across each 2x2 pixel quad, merged
	# via QuadReadAcrossX/Y/Diagonal -- ~4x fewer atlas samples per pixel for
	# the search, same total 16-tap coverage. New/unverified, hence a
	# runtime A/B switch rather than replacing the original outright.
	int quad_blocker_search;
	# 0 = single blur pass (uses the RTX-verified distance when the
	# verification ray hit something, VSM's own estimate otherwise -- cheaper,
	# one 16-tap blur per pixel). 1 = blur BOTH distances and take min() of
	# the two resulting shadow values (pricier -- an extra 16-tap blur
	# whenever the ray hits -- but avoids the bright spots a single blended
	# estimate produced between overlapping penumbras). Only read when
	# VsmRtxVerify is enabled.
	int rtx_dual_blur;
	# Runtime A/B switch (Phase 5.18 Part A) for the min/max Hi-Z
	# classification vsm_search_blocker does against VSMPageHiZ before
	# running its 16-tap search -- nonzero = skip the search entirely for
	# pixels the pyramid alone can already answer confidently (see
	# vsm_search_blocker's own comment). Only read by VSM_BlockerSearch's
	# own dispatch; VSM_Combine never consults it.
	int hiz_blocker_classify;
	# Runtime A/B switch: nonzero = vsm_search_blocker discards a candidate
	# tap whose reconstructed world position falls behind the receiver's own
	# tangent plane (point+normal hemisphere) before counting it as a
	# blocker. Fixes a near-two-sided-floor-at-grazing-sun case: a nearly
	# coplanar/self-referential "blocker" the shadow map records can pass
	# the plain sampled>pos_l.z depth test while being geometrically behind
	# the receiving surface, not a real occluder for it at all. Only read by
	# VSM_BlockerSearch's own dispatch (vsm_search_blocker's tap loop);
	# vsm_classify_blocker's Hi-Z classification and VSM_Combine never
	# consult it.
	int hemisphere_cull_blocker;
	# Non-penumbra fallback only (VSM_Combine's own combine_result reads
	# this; the penumbra-on path's equivalent views live entirely in
	# VSM_DebugClassifyOverlay/VSM_DebugTileOverlay.hlsl instead, selected
	# by which dedicated PSO gets dispatched, not by this field -- see
	# VSM.cpp's m_debugoverlay_render). PageGrid: colors every pixel by
	# which clipmap level and which page WITHIN that level it resolved to,
	# one flat color per level, darkened on a checkerboard by (page_x+
	# page_y) parity so page SEAMS show up as a visible brightness step.
	# RtxReference: bypasses get_shadow_vsm_simple entirely and displays
	# RTXShadow's own (denoised) full-RT shadow mask directly as grayscale
	# -- a reference to compare VSM's quality/performance against (see
	# VSMLighting's rtx_shadow_mask field). HizClassify has no meaning here
	# (VSM_Combine has no Hi-Z classification of its own to visualize) --
	# VSM.cpp's own constants upload never sends that value to this field.
	VSMDebugView debug_view = None;
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
# dirty-page-per-mip loop. src/dst_mip are both narrowed, single-mip,
# [Barrier = ALL] views of the SAME VSM_PageHiZ resource -- src is RW (a
# UAV), not an SRV, DELIBERATELY: with src+dst_mip on two different
# subresources (mip N vs N+1) of one [Barrier = ALL]-tracked (whole-
# resource, one layout) texture, an SRV read + a UAV write in the SAME
# dispatch need two INCOMPATIBLE layouts simultaneously -- confirmed live
# via GPU-Based Validation (#1358, "expected SRV layout, found
# UNORDERED_ACCESS") once GBV was turned on to chase an unrelated VSM tile
# flicker. Keeping src a UAV too means the whole resource only ever needs
# UNORDERED_ACCESS for this entire loop, so there is no layout conflict to
# resolve in the first place.
[Bind = DefaultLayout::Instance0]
struct VSMDownsampleHiZBatch
{
	# Both sides are narrowed to exactly one mip across physical_page_count
	# slices, and this runs once per mip per frame -- the single worst
	# subresource-expansion site in the frame. See VSMCopyPageDepthBatch.
	# float2: .x = running MIN (farthest), .y = running MAX (closest) -- see
	# VSMPageHiZ's own comment.
	[Barrier = ALL] RWTexture2DArray<float2> src;
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
}

# Phase 5.18 Part A follow-up (take 4): groupshared tile classification,
# mirroring VoxelGIGraph's FrameClassification/FrameClassificationInitDispatch
# pattern -- one 16x16 group per screen tile, each thread runs the cheap
# Hi-Z classify test (vsm_classify_blocker, VSM_impl_search.hlsl) and a
# groupshared reduction decides the WHOLE tile's bucket:
#   - lit_tiles:    every pixel confidently lit
#   - dark_tiles:   every pixel confidently dark
#   - search_tiles: anything else -- at least one genuinely ambiguous pixel,
#                   OR a mix of confidently-lit and confidently-dark pixels
#                   with none ambiguous (a hard boundary with no true
#                   penumbra pixel in this tile -- can't go in either
#                   uniform bucket, since stage 3's full-lit/full-shadow
#                   PSOs write a flat value with no per-pixel check).
[Bind = DefaultLayout::Instance1]
struct VSMBlockerTilesAppend
{
	AppendStructuredBuffer<uint2> lit_tiles;
	AppendStructuredBuffer<uint2> dark_tiles;
	AppendStructuredBuffer<uint2> search_tiles;
}

# Read-side counterpart of VSMBlockerTilesAppend's three fields -- same
# three underlying buffers, each bound as a plain StructuredBuffer instead
# of an AppendStructuredBuffer. One generic single-field struct, reused by
# VSM_BlockerSearch (search_tiles) and all three of stage 3's PSOs (one
# list each) -- mirrors VoxelGI's own TilingPostprocess/Tiling struct,
# reused the same way across VoxelBlur/VoxelIndirectFilter/DenoiserHistoryFix.
[Bind = DefaultLayout::Instance1]
struct VSMTileListRead
{
	StructuredBuffer<uint2> tiles;
}

ComputePSO VSMBlockerClassify
{
	root = DefaultLayout;

	[EntryPoint = CS_BLOCKER_CLASSIFY]
	compute = VSM_BlockerClassify;
}

# Stage 2 follow-up: even a tile stage 1 bucketed as search_tiles can turn
# out, after the REAL per-pixel search actually runs, to need no further
# work at all -- every one of its 256 pixels individually resolved to a lit
# sentinel (no blocker found), the same "fully visible" outcome stage 1's
# cheap Hi-Z test just couldn't prove up front. Appending such a tile
# straight to a second "confirmed lit" list lets stage 3 skip launching the
# expensive shadow-blur PSO for it entirely, on top of the launch stage 1
# already skips for lit_tiles/dark_tiles.
#
# NOT simply "zero real blockers found" -- a tile can ALSO have a mix of
# confidently-lit and confidently-dark pixels with zero real blockers (the
# same hard-boundary-no-penumbra case that forced it into search_tiles in
# the first place, see VSM_BlockerClassify's own comment). Flat-filling such
# a tile as lit would silently wipe out its dark pixels. So the verdict
# tracks BOTH "any pixel resolved dark" and "any pixel found a real
# blocker" -- only when NEITHER happened is confirmed_lit_tiles safe;
# otherwise the tile goes to blur_tiles, which stage 3's shadow-blur PSO
# still resolves correctly per-pixel (including pixels that individually
# turned out lit or dark within it) via VSM_BlockerSearchResult's existing
# sentinel decode -- no actual tap-loop blur runs for those pixels either,
# just narrower coverage than launching over the whole original
# search_tiles would have.
[Bind = DefaultLayout::Instance5]
struct VSMSearchVerdictAppend
{
	AppendStructuredBuffer<uint2> confirmed_lit_tiles;
	AppendStructuredBuffer<uint2> blur_tiles;
}

# Stage 2's own output -- VSM_BlockerSearch (INDIRECT, over search_tiles
# only) writes the raw per-ambiguous-pixel blocker-search result here
# instead of into a texture VSM_Combine reads directly: keeps "raw search
# output for ambiguous pixels only" separate from stage 3's own final
# resolved shadow value, so the two meanings never get confused the way a
# single shared texture's did across earlier attempts. Instance3: free for
# this PSO (VSMPageTableData claims Instance3 too, but for a completely
# different PSO -- mesh_shader_vsm.hlsl -- no collision, a given PSO only
# ever binds the structs its own shader references).
#
# uint4, not float4: x = asuint(world_delta), or asuint(-1.0)/asuint(-2.0)
# as the confident_lit/confident_dark sentinels for a pixel that resolved
# individually within an otherwise-ambiguous tile (world_delta is otherwise
# always >=0 by construction); y/z = asuint(best_tc.x)/asuint(best_tc.y);
# w = best_slot directly (already a uint).
[Bind = DefaultLayout::Instance3]
struct VSMBlockerSearchOutput
{
	RWTexture2D<uint4> blocker_search_result;
}

# Stage 3's own I/O -- reads stage 2's raw search output (SRV, same
# underlying resource as VSMBlockerSearchOutput's RW field above, just this
# PSO only ever reads it) and writes the FINAL resolved shadow scalar
# (plain float, not a packed uint4 -- blur has already happened by the time
# this is written, nothing left to decode). Same Instance3 slot as
# VSMBlockerSearchOutput above -- different PSO, no collision.
[Bind = DefaultLayout::Instance3]
struct VSMShadowResolveIO
{
	Texture2D<uint4> blocker_search_result;
}

# VSM_Combine only ever runs now when use_vsm_penumbra is off (see that
# PassNode's own comment) -- the penumbra-on case moved entirely into stage
# 3 (VSM_ShadowResolve), which writes ResultTexture directly instead of an
# intermediate scalar this PSO used to sample. So there's only ever one
# real permutation of this PSO left; the old VsmPenumbra define (and
# combine_result's #ifdef VSM_PENUMBRA branch in VSM.hlsl) is gone rather
# than kept as a permanently-unreachable "off" path.
ComputePSO VSMApplyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_RESULT]
	compute = VSM;
}

# Blocker-search extraction: INDIRECT dispatch (Phase 5.18 Part A follow-up:
# groupshared tile classification) over only search_tiles -- the tiles
# VSM_BlockerClassify actually bucketed as needing real work. Runs the wide,
# many-tap PCSS blocker search (VSM_impl_search.hlsl's vsm_search_blocker),
# writing its raw result to VSMBlockerSearchOutput's blocker_search_result
# for stage 3's shadow-blur PSO to read back -- see that struct's own
# comment. No VsmRtxVerify define here: RTX ray-firing happens entirely in
# stage 3's shadow-blur PSO now (it's what actually runs the PCF blur the
# ray-corrected distance feeds into), not here.
ComputePSO VSMBlockerSearchCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_BLOCKER_SEARCH]
	compute = VSM_BlockerSearch;
}

# Stage 3: three PSOs sharing one file (VSM_ShadowResolve.hlsl), one
# PassNode, one render() -- see VSM_ShadowResolve's own PassNode comment for
# why all three must be issued from the same render() (mirrors VoxelGI's
# VoxelCombine issuing its own blur+blur2 exec_indirects together).
ComputePSO VSMFullLit
{
	root = DefaultLayout;

	[EntryPoint = CS_FULL_LIT]
	compute = VSM_ShadowResolve;
}

ComputePSO VSMFullShadow
{
	root = DefaultLayout;

	[EntryPoint = CS_FULL_SHADOW]
	compute = VSM_ShadowResolve;
}

ComputePSO VSMShadowBlur
{
	root = DefaultLayout;

	[EntryPoint = CS_SHADOW_BLUR]
	compute = VSM_ShadowResolve;

	# Once the blocker search (stage 2) finds a blocker, fires one RayQuery
	# toward the sun to verify/correct its distance against the real BVH --
	# shadow maps only record the front-most surface per texel, so a closer
	# blocker can exist without ever being rasterized where the search
	# looked. Needs RTX hardware; gated at runtime in VSM.cpp, not just by
	# this define, since VSM must keep working correctly without it. Moved
	# here from VSMApplyCompute -- this is the PSO that now actually runs
	# the PCF blur the ray-corrected distance feeds into.
	[rename = VSM_RTX_VERIFY]
	[CS, nullable]
	define VsmRtxVerify;
}

# Debug overlay (see VSM_DebugClassifyOverlay's own PassNode comment) --
# four trivial PSOs sharing one file, each an indirect dispatch over one
# tile list (two from stage 1, two from stage 2's own post-search verdict),
# painting a flat color directly onto the already-shaded ResultTexture.
ComputePSO VSMDebugOverlayLit
{
	root = DefaultLayout;

	[EntryPoint = CS_OVERLAY_LIT]
	compute = VSM_DebugTileOverlay;
}

ComputePSO VSMDebugOverlayDark
{
	root = DefaultLayout;

	[EntryPoint = CS_OVERLAY_DARK]
	compute = VSM_DebugTileOverlay;
}

ComputePSO VSMDebugOverlayConfirmedLit
{
	root = DefaultLayout;

	[EntryPoint = CS_OVERLAY_CONFIRMED_LIT]
	compute = VSM_DebugTileOverlay;
}

ComputePSO VSMDebugOverlayBlur
{
	root = DefaultLayout;

	[EntryPoint = CS_OVERLAY_BLUR]
	compute = VSM_DebugTileOverlay;
}

# Moved here from VSM_Combine's own combine_result (VSM.hlsl) now that stage
# 3 (VSM_ShadowResolve) writes ResultTexture directly and VSM_Combine no
# longer runs at all when use_vsm_penumbra is on -- see this PassNode's own
# comment for why. Both are full-screen, not tile-list-driven (these debug
# views don't care which classify bucket a pixel landed in), unlike every
# other entry point in this file.
ComputePSO VSMDebugOverlayPageGrid
{
	root = DefaultLayout;

	[EntryPoint = CS_OVERLAY_PAGE_GRID]
	compute = VSM_DebugTileOverlay;
}

ComputePSO VSMDebugOverlayRtxReference
{
	root = DefaultLayout;

	[EntryPoint = CS_OVERLAY_RTX_REFERENCE]
	compute = VSM_DebugTileOverlay;
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

# Phase 5.19: alpha-cutout materials need a real pixel shader (clip() on
# opacity) in the VSM depth pass, or they'd cast a fully solid shadow
# regardless of opacity -- plain VSMDepthDraw above has no pixel shader at
# all ([Erase] pixel = null), by design, since the overwhelming majority of
# materials are opaque and shouldn't pay for one. This is the per-material
# variant: same mesh/amplification shaders (mesh_shader_vsm doesn't care
# which depth PSO ends up bound), but `pixel` is left for
# materials::PipelinePasses to override with the material's own compiled PS
# entry point (UniversalMaterial.hlsl's PS -- the SAME entry point
# depth_draw/gbuffer already use; a GBuffer-returning PS bound to a PSO
# with no RTVs just has its color outputs discarded, exactly like DepthDraw
# already relies on). Only ever compiled for materials whose opacity graph
# is actually driven (see PipelinePasses' constructor) -- opaque materials
# never get one of these at all, keeping VSMDepthDraw the common case.
[Base]
GraphicsPSO VSMDepthDrawMaterial
{
	root = DefaultLayout;

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
# MeshInstanceInfo is Instance2, MaterialInfo is DefaultLayout::MaterialData
# (declared AFTER Instance0-5/Raytracing in defaultlayout.sig, so its root
# index is higher than all three Instance ones) -- material_cb must come
# last of the pointer fields, matching meshrender.sig's CommandData's own
# order, or CreateCommandSignature fails with "Root parameter {slots,
# offset} must be increasing" (confirmed the hard way, twice now).
#
# Phase 5.19: material_cb is populated for EVERY entry, not just alpha-
# cutout ones -- vsm_gather_dispatch.hlsl's CS (default/opaque list) sets it
# too, even though VSMDepthDraw's shader (pixel=null) never reads it, since
# both the default and material-bucketed lists share this one struct/one
# command signature. Harmless for the opaque case; required for
# VSMDepthDrawMaterial's real pixel shader (UniversalMaterial.hlsl's PS),
# which needs MaterialInfo bound to sample its textures at all -- its
# absence here (before this fix) surfaced as "Possible null slot
# MaterialInfo" followed by a null-PSO crash.
[IndirectCommand]
struct VSMDispatchCommandData
{
	VSMPageBatch* page_batch_cb;
	MeshInfo* mesh_cb;
	MeshInstanceInfo* meshinstance_cb;
	MaterialInfo* material_cb;
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

# Phase 5.19: alpha-cutout material routing. A different PSO from
# VSMGatherDispatch above (its own Instance1 binding, no collision -- only
# one of the two is ever bound at once), same file/mesh-vs-level test, but
# routes matching entries into one of up to 8 per-material-pipeline bucket
# lists instead of the single default list -- mirrors meshrender.sig's
# GatherPipeline (pip_ids[2]/commands[8]) and gather_pipeline.hlsl's
# get_index exactly, just producing VSMDispatchCommandData instead of
# CommandData. VSM.cpp runs this once per batch of <=8 distinct transparent
# material pipelines actually present in the scene (same batching loop
# shape as MeshRenderer::render_meshes), reusing 8 fixed VSM-owned bucket
# buffers across batches -- see VSM.cpp's own comment at the call site.
[Bind = DefaultLayout::Instance1]
struct VSMGatherDispatchMaterialData
{
	StructuredBuffer<VSMLevelDispatchInfo> levels;
	uint level_count;
	float4x4 light_view;
	# This batch's up to 8 distinct transparent-material pipeline ids
	# (materials::Pipeline::get_id()), packed the same uint4[2] way
	# GatherPipeline.pip_ids is. A slot beyond this batch's real count is
	# set to 0xFFFFFFFF (never a real pipeline id) so it can't spuriously
	# match.
	uint4 material_pip_ids[2];
	AppendStructuredBuffer<VSMDispatchCommandData> material_commands[8];
}

ComputePSO VSMGatherDispatchMaterial
{
	root = DefaultLayout;

	[EntryPoint = CS_MATERIAL]
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
	# Phase 5.19: read here too (not just by VSM_GatherDispatch that wrote
	# it) -- this pass's render() also dispatches VSMGatherDispatchMaterial
	# (CS_MATERIAL) per batch of transparent-material pipelines, immediately
	# followed by that batch's own material-PSO draw, so the whole alpha-
	# cutout gather+draw cycle stays in one place instead of splitting across
	# two PassNodes and racing the shared 8-bucket pool (see VSM.cpp's own
	# comment at the call site for why it can't split).
	StructuredBuffer<VSMLevelDispatchInfo> VSM_LevelDispatchInfo;
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

# Stage 1 (Phase 5.18 Part A follow-up, take 4): groupshared tile
# classification -- see VSMBlockerTilesAppend's own comment for the
# per-tile verdict logic. Writes NOTHING pixel-shaped: only the three tile-
# position lists (VSMBlockerTilesAppend). The matching indirect dispatch
# args for stage 2/stage 3 are no longer built by a shader here at all --
# VSM.cpp's render() copies each list's own AppendStructuredBuffer counter
# directly into a VSM-owned DispatchArguments buffer's ThreadGroupCountX
# (same pattern as PassDefaults.cpp's ShadowsFlowNode/Graph::indirect_dispatch_args),
# in the SAME render() as the append above -- still required to stay
# together (an AppendStructuredBuffer's hidden GPU counter isn't reliably
# barrier-tracked by FrameGraph's normal resource-view dependency system
# across a PassNode boundary, confirmed live earlier this session), just via
# a copy instead of a dispatch now.
[Compute]
PassNode VSM_BlockerClassify
{
	GBuffer gbuffer;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	Texture VSM_PageHiZ;
	[Write] StructuredBuffer<uint2> VSM_LitTiles;
	[Write] StructuredBuffer<uint2> VSM_DarkTiles;
	[Write] StructuredBuffer<uint2> VSM_SearchTiles;
}

# Stage 2: INDIRECT dispatch over VSM_SearchTiles only -- the tiles stage 1
# bucketed as needing real work. Runs the wide, many-tap PCSS blocker search
# (VSM_impl_search.hlsl's vsm_search_blocker), writing its raw result to
# VSM_BlockerSearchResult (VSMBlockerSearchOutput's own field) for stage
# 3's shadow-blur PSO to read back -- NOT to a texture VSM_Combine reads
# directly any more (see VSMBlockerSearchOutput's own comment for why).
#
# Phase 5.18 Part A: also reads VSM_PageHiZ -- vsm_search_blocker does a
# cheap min/max classification against the receiver's own page pyramid
# before running the expensive 16-tap search, skipping the search entirely
# for pixels the pyramid alone can already answer confidently within an
# otherwise-ambiguous tile (see VSMPageHiZ's own comment for the two
# channels' meaning). This is why VSM_HiZRebuild moved to run immediately
# before stage 1 in test.sig's listing -- the pyramid both stage 1 and
# stage 2 read must be this frame's freshly-rebuilt one, not last frame's.
[Compute]
PassNode VSM_BlockerSearch
{
	GBuffer gbuffer;
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	Texture VSM_PageHiZ;
	Texture BlueNoise;
	StructuredBuffer<uint2> VSM_SearchTiles;
	[Write] Texture VSM_BlockerSearchResult;
	# Stage 2's own post-search verdict lists -- see VSMSearchVerdictAppend's
	# own comment. VSM_ConfirmedLitTiles feeds a second full-lit dispatch in
	# stage 3; VSM_BlurTiles replaces VSM_SearchTiles as what stage 3's
	# shadow-blur PSO actually dispatches over. Their indirect dispatch args
	# (and VSM_SearchTiles' own, this pass's own indirect dispatch source)
	# are VSM-owned buffers now, not FrameGraph fields -- see
	# VSM_BlockerClassify's own comment for why.
	[Write] StructuredBuffer<uint2> VSM_ConfirmedLitTiles;
	[Write] StructuredBuffer<uint2> VSM_BlurTiles;
}

# Stage 3: three PSOs (VSMFullLit/VSMFullShadow/VSMShadowBlur), ONE
# PassNode, ONE render() -- see this session's own root-cause finding
# (VoxelGIGraph.cpp's VoxelCombine issuing its own blur+blur2 exec_indirects
# together, never split across separate PassNodes) for why: every write to
# a shared output resource, however many indirect dispatches it takes, must
# come from one PassNode's render(), or FrameGraph's dependency resolution
# doesn't reliably make every writer's output visible to the resource's
# other consumers. ResultTexture is written disjointly by all three PSOs
# (lit_tiles -> flat lit, dark_tiles -> flat black, search_tiles -> the real
# PCF blur read from VSM_BlockerSearchResult) -- full coverage by
# construction, same as the three tile lists are disjoint by construction.
#
# Used to write an intermediate VSM_ShadowResult scalar for VSM_Combine's
# own separate full-screen pass to read and apply -- collapsed into one
# step (each PSO now does the full PBR combine itself and writes
# ResultTexture directly) once it was clear these dispatches already cover
# every screen pixel exactly once, making VSM_Combine's own full-screen
# dispatch (plus the shadow-mask texture's read/write round trip) pure
# overhead whenever use_vsm_penumbra is on. VSM_Combine still exists for the
# non-penumbra fallback (get_shadow_vsm_simple, no tile pipeline to
# piggyback on) -- see its own PassNode comment.
[Compute]
PassNode VSM_ShadowResolve
{
	GBuffer gbuffer;
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	Texture BlueNoise;
	StructuredBuffer<uint2> VSM_LitTiles;
	StructuredBuffer<uint2> VSM_DarkTiles;
	# Stage 2's own post-search verdict lists, replacing VSM_SearchTiles here
	# -- see VSMSearchVerdictAppend's own comment. Confirmed-lit gets a
	# second cheap full-lit dispatch; blur_tiles is the (usually smaller)
	# real target for the shadow-blur PSO. All four lists' indirect dispatch
	# args are VSM-owned buffers now (see VSM_BlockerClassify's own comment),
	# not FrameGraph fields -- render() reads them straight off `this`.
	StructuredBuffer<uint2> VSM_ConfirmedLitTiles;
	StructuredBuffer<uint2> VSM_BlurTiles;
	Texture VSM_BlockerSearchResult;
	[Write] Texture ResultTexture;
}

# Only runs when use_vsm_penumbra is OFF now (see m_combine_setup's own
# early-out) -- the penumbra-on case is handled entirely by stage 3
# (VSM_ShadowResolve) writing ResultTexture directly, since its four
# dispatches already cover every pixel and there's no tile pipeline for
# this pass to still add value on top of. Kept for the non-penumbra
# fallback (get_shadow_vsm_simple, a plain fixed 3x3 hardware-PCF full-
# screen pass with no tile lists to dispatch over) and nothing else.
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
	[Write] Texture ResultTexture;
}

# Debug view (VSM.ixx's use_vsm_debug_hiz_classify), moved out of VSM_Combine
# itself: the earlier in-line version approximated "which bucket did this
# pixel come from" by checking whether the FINAL shadow value happened to
# equal exactly 1.0/0.0 -- a lossy postfactum guess (a genuinely blurred
# result can also land on exactly 0 or 1), not the real classification data.
# This pass instead reads the real tile lists directly and paints a flat
# overlay color onto the ALREADY-shaded ResultTexture, on top of
# VSM_Combine's real output -- green=lit_tiles (stage 1), blue=dark_tiles
# (stage 1), cyan=confirmed_lit_tiles (stage 2's post-search confirmation).
# blur_tiles gets PER-PIXEL treatment instead of a flat color: it reads
# VSM_BlockerSearchResult directly (the same packed data CS_SHADOW_BLUR
# itself decodes) and paints a DARKER green/blue for pixels that resolved
# via a sentinel even though their tile still had to dispatch (stage 2's
# per-pixel-level optimization within an otherwise-dispatched tile),
# leaving pixels that ran the real tap-loop blur untouched so the real
# blurred shadow shows through -- see VSM_DebugTileOverlay.hlsl's own
# comment on CS_OVERLAY_BLUR.
#
# Also now the only home (when use_vsm_penumbra is on) for two debug views
# that used to live in VSM_Combine's own combine_result: use_vsm_debug_page_
# grid and use_vsm_debug_rtx_reference. VSM_Combine no longer runs at all
# when penumbra is on (see its own PassNode comment), so this pass's own
# early-out (m_debugoverlay_setup) widened to cover all three debug toggles,
# not just use_vsm_debug_hiz_classify -- otherwise those two would go dark
# the moment penumbra mode is the active path. gbuffer/ShadowMask are only
# for those two (full-screen, not tile-list-driven, unlike everything else
# in this PassNode) -- see VSM_DebugTileOverlay.hlsl's own CS_OVERLAY_PAGE_
# GRID/CS_OVERLAY_RTX_REFERENCE.
[Compute]
PassNode VSM_DebugClassifyOverlay
{
	GBuffer gbuffer;
	StructuredBuffer<uint2> VSM_LitTiles;
	StructuredBuffer<uint2> VSM_DarkTiles;
	StructuredBuffer<uint2> VSM_ConfirmedLitTiles;
	StructuredBuffer<uint2> VSM_BlurTiles;
	Texture VSM_BlockerSearchResult;
	# Same resource RTXShadow writes / PSSM_Combine reads -- see
	# VSMLighting's rtx_shadow_mask field. Not [Write]: only ever read, for
	# use_vsm_debug_rtx_reference.
	Texture ShadowMask;
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
