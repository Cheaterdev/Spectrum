# DDGI probe-volume GI (see [[project-ddgi]] planning notes) -- a budgeted,
# camera-following grid of world-space probes caching indirect irradiance,
# modeled on Ubisoft's "Raytracing the World of Assassin's Creed Shadows"
# (Advances 2025). Gets multi/infinite-bounce GI by feeding each probe's own
# *previous frame* irradiance back in as incoming light when relighting it
# (talk: "using values from the previous frame ... multi-bounce global
# illumination"), instead of tracing more bounces per pixel per frame --
# IndirectRTX (voxel.sig) still does exactly one ray per pixel.
#
# v1 scaffold: 5-cascade grid (each level double the previous one's probe
# spacing, all centered/recentered on the camera, same shape AC Shadows'
# talk describes -- "5 cascades 16x16x8"), naive round-robin probe budget
# instead of AC Shadows' 3-step greedy priority scheme, no relocation/
# classification, no specular probes. All deferred refinements are called
# out at their would-be call site below.
#
# Cascades share ONE physical atlas per texture (5x as wide) and ONE probe
# buffer (5x as many entries), using this engine's [Multiple=N] PassNode
# mechanism (same one PSSM_Cascade, pssm.sig, uses for shadow cascades):
# DDGIProbeSelect/Trace/Convolve below are each [Multiple=5], instance 0
# creates the shared (wide) resources and every instance just needs them
# (see pass.jinja's own comment on [Optional] + [Size] together, quoted at
# PSSM_Cascade). Each instance's own `data.pass_index` is its cascade index;
# DDGIInfo::cascade_info carries the per-cascade probe-buffer offset and
# atlas X-offset into the shared wide resources (HLSL can't reach the
# generated Constants:: namespace to compute these itself, same reasoning
# atlas_info.x/DDGI_ProbeTexelSize already documents).
const DDGI_CascadeCount = 5;

# Bit values for DDGIInfo::flags.z (below) -- a real SIG enum instead of
# hand-typed magic numbers (1u/2u/4u/8u) at every call/check site, C++ and
# HLSL alike. Explicit power-of-two values since these are combined as a
# bitmask (DDGIGraph.cpp OR's the ones whose toggle is on into flags.z),
# not picked one-at-a-time the way every other SIG enum in this codebase is
# (a dropdown/[Optional] condition) -- there's no built-in "this enum is a
# flag set" concept, so the bitwise OR/AND still has to be written by hand
# at each site; only the magic numbers themselves are replaced.
enum DDGIControlFlags
{
	CullCoarsestCascade = 1;
	DisableResidencyCulling = 2;
	JitterRays = 4;
	DisableTraceFeedback = 8;
}

# Single-select (not a bitmask, unlike DDGIControlFlags above -- sequential
# values, not powers of two), mirrored from DDGIGraph.cpp's Variable<
# DDGIOcclusionMode> "Occlusion test" combo box. Packed into DDGIInfo::
# flags.z bits 4-5 (DDGIControlFlags only uses bits 0-3) -- see that field's
# own comment for the extraction shift/mask. Controls how ddgi_sample_irradiance
# (ddgi_sample.hlsl) weights each of its 8 trilinear probe corners against
# occlusion between the probe and the shading point:
#   NoOcclusionTest -- skip the test entirely (fastest, most light leaking
#     through walls/thin occluders -- an A/B baseline more than something to
#     actually ship with).
#   ProbeDepthTest -- read the probe's own stored visibility texel (mean hit
#     distance in the direction from probe to shading point, same moments
#     DDGIProbeTrace/DDGIProbeConvolve already write) and compare directly:
#     is the shading point closer than what the probe last saw in that
#     direction? A plain depth-map-style comparison, not the statistical
#     chebyshev/variance test this replaced -- cheap (one more texture read,
#     no extra ray) and avoids chebyshev's own well-known "light bleeding"
#     failure mode from bilinearly interpolating raw moments before deriving
#     variance.
#   RTXRay -- fire an actual inline (RayQuery) ray per corner between the
#     shading point and the probe (ddgi_probe_visibility_ray, ddgi_sample.hlsl)
#     -- correct, but real added cost.
enum DDGIOcclusionMode
{
	NoOcclusionTest = 0;
	ProbeDepthTest = 1;
	RTXRay = 2;
}

# Single-cascade grid size, shared between DDGI.ixx's grid bookkeeping and
# every [Size=...] below -- one source of truth, same reasoning as vsm.sig's
# MaxLevels/VSM_PagesPerLevelSide. 4x/dimension over the original v1 scaffold
# (was 16x8x16) -- affordable now that residency culling (DDGIProbeResidencyMark)
# means most of this grid is never actually traced/convolved, only stored.
const DDGI_ProbeCountX = 64;
const DDGI_ProbeCountY = 32;
const DDGI_ProbeCountZ = 64;
const DDGI_ProbeCount = `Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeCountY * Constants::DDGI_ProbeCountZ`;

# Octahedral atlas texel budget per probe. v1 uses the same texel size for
# radiance/irradiance/visibility for scaffold simplicity -- AC Shadows uses
# different sizes per buffer (10x10 radiance, 5x5 irradiance, 20x20
# visibility, all with borders) to trade memory for filtering quality; right-
# sizing each independently is a follow-up, not a correctness requirement.
const DDGI_ProbeTexelSize = 8;
# Atlas layout: the 2D plane holds (probe_x, probe_z) only -- probe_y AND
# the cascade both live in the ARRAY dimension instead (DDGI_AtlasArraySlices,
# below), via a Texture2DArray. This is why: at 4x/dimension, folding
# (probe_x, probe_z) alone into width like the original v1 scaffold did
# already reaches 64*64*8 = 32768px, over D3D12's 16384 max texture
# dimension -- before even multiplying by DDGI_CascadeCount the way the v1
# scaffold additionally did. Moving probe_y AND cascade into array slices
# instead keeps both the plane (512x512) and the array count (32*5=160)
# comfortably within limits. See DDGIProbes' ddgi_atlas_origin()/
# ddgi_atlas_array_slice() helpers below for the addressing this implies.
const DDGI_AtlasWidth  = `Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeTexelSize`;
const DDGI_AtlasHeight = `Constants::DDGI_ProbeCountZ * Constants::DDGI_ProbeTexelSize`;
const DDGI_AtlasArraySlices = `Constants::DDGI_ProbeCountY * Constants::DDGI_CascadeCount`;

# Fixed ray budget PER PROBE, decoupled from DDGI_ProbeTexelSize -- v1 traced
# exactly one ray per eventual octahedral output texel (64 = 8x8), which
# tied ray count to atlas resolution for no real reason: DDGIProbeTrace now
# fires DDGI_ProbeRayCount spherical-fibonacci directions per probe
# (ddgi_sphere_fibonacci, octahedral.hlsl -- deterministic from index/count
# alone, no direction table needed) into a flat per-ray buffer
# (DDGI_ProbeRayRadiance, below), and DDGIProbeConvolve cosine-weight-
# resamples that same fixed ray set into all DDGI_ProbeTexelSize^2 output
# texels instead of reading a 1:1 traced value per texel -- same "real DDGI
# implementations decouple ray count from texel resolution" plan item this
# file's own top comment already called out. 32 (half of the old 64) is a
# starting point: fewer independent directional samples per probe means a
# thin occluder is statistically less likely to have a ray land on it, so
# watch for increased light leaking relative to before if this needs
# retuning -- there's no free lunch here, just spending the same ray budget
# on trading (texel-resolution-tied count) for (a number you can actually
# tune independently of the output atlas).
const DDGI_ProbeRayCount = 32;

# Per-frame probe-update budget (DDGI.ixx's Variable<int> probes_per_frame_budget
# mirrors into DDGISelectors below); this constant is only the storage-independent
# upper bound used to size nothing in particular yet -- kept for parity with
# vsm.sig's MaxDispatchEntries pattern in case a future selection pass needs a
# GPU-side counted append buffer sized off it.
const DDGI_MaxProbesPerFrame = `Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeCountY`;

# Mirrored once per frame by DDGI::update_frame() (DDGIGraph.cpp), same
# reasoning as VoxelInfo (voxel.sig): grid_min is the world-space corner of
# probe (0,0,0), probe_spacing the world-space distance between adjacent
# probes along each axis. rays_per_probe.x is the real per-probe ray count
# now (DDGI_ProbeRayCount, mirrored rather than read directly from HLSL's
# generated Constants:: namespace the same reason atlas_info.x is below) --
# DDGIProbeTrace's raygen dispatches exactly this many spherical-fibonacci
# rays per probe, decoupled from the output atlas's own texel resolution
# (see DDGI_ProbeRayCount's own comment for why).
# rays_per_probe.y/.z carry this cascade's own stagger_k/stagger_bucket (see
# DDGIGraph.cpp's g_ddgi_stagger_k/g_ddgi_stagger_bucket) -- which rotating
# 1/stagger_k-sized subset of this cascade's probes is due for retrace this
# frame, per DDGIProbeResidencyMark's own compaction gate below. .w remains
# unused padding. atlas_info.x
# is DDGI_ProbeTexelSize -- HLSL has no access to the generated Constants::
# namespace (C++-only), so shaders that need to convert an atlas texel
# coordinate to a probe grid coordinate (DDGIProbes::ddgi_atlas_probe_coord/
# ddgi_atlas_local_uv, below) read it from here instead. cascade_info.x is
# this cascade's linear offset into the (DDGI_CascadeCount-times-larger)
# shared DDGI_Probes buffer; .y is its array-slice offset into the shared
# (DDGI_AtlasArraySlices-deep) atlas array textures -- this cascade's probes
# occupy slices [.y, .y+DDGI_ProbeCountY), see ddgi_atlas_array_slice()
# below; .z is the cascade index itself (informational/debug only). All
# precomputed in C++ (DDGIGraph.cpp's ddgi_make_info) since HLSL can't reach
# Constants:: to derive them itself, same reasoning atlas_info.x already
# documents. .w is
# 1 for the coarsest cascade (DDGI_CascadeCount-1), 0 otherwise -- the
# coarsest level is exempt from residency culling (nowhere further to fall
# back to, so it must stay fully resident as the guaranteed-valid floor
# other cascades lean on), see DDGIProbeResidencyMark's own comment.
[Bind = DefaultLayout::Instance0]
struct DDGIInfo
{
	# .xyz = this cascade's grid minimum corner (world space). .w =
	# depth_test_bias, mirrored from DDGIGraph.cpp's Variable<float> "Depth
	# test bias" -- ddgi_probe_depth_test's own comment (ddgi_sample.hlsl)
	# explains why DDGIOcclusionMode::ProbeDepthTest needs a bias at all;
	# this is that bias's own SCALE, expressed as a fraction of this
	# cascade's own probe spacing (so the same fraction gives a
	# proportionally larger bias in coarser, wider-spaced cascades instead
	# of a single world-space constant being too tight or too loose
	# depending on which cascade is sampled).
	float4 grid_min;
	# .xyz = probe spacing for this cascade. .w = feedback_strength, mirrored
	# from DDGIGraph.cpp's Variable<float> "Feedback strength" -- scales the
	# multi-bounce term DDGIProbeTrace's own hit-point sample adds on top of
	# direct-lit shading (ddgi_probe_trace.hlsl), separate from flags.x
	# (use_fallback, the final per-pixel screen term's own master on/off).
	# Two probes whose traced rays land near each other feed each other's
	# irradiance back across frames (see [[project-ddgi]] planning notes) --
	# this scalar is a knob on how strongly that loop reinforces itself
	# before it's fully squashed by the built-in albedo attenuation each hop
	# already provides.
	float4 probe_spacing;
	uint4 probe_counts;
	uint4 rays_per_probe;
	uint4 atlas_info;
	uint4 cascade_info;
	# .x = use_fallback (0/1) -- master on/off for whether the probe-volume
	# term actually gets added anywhere it's sampled (DDGIProbeTrace's own
	# self-feedback AND TraceIndirectDiffuse's per-pixel term), mirrored from
	# DDGIGraph.cpp's Variable<bool> "Use probe fallback". Lets the DDGI
	# system keep tracing/convolving (so re-enabling doesn't start from cold)
	# while its contribution is excluded from actual lighting, for an A/B
	# comparison against plain 1-bounce RTX.
	# .y = use_indirect_dispatch (0/1), mirrored from DDGIGraph.cpp's
	# Variable<bool> "Use indirect DispatchRays" -- selects which addressing
	# scheme ddgi_probe_trace.hlsl's raygen shader uses (DispatchRaysIndex().xy
	# directly into the atlas vs. a compacted linear index through
	# DDGIProbeTraceData::compacted_list), matching whichever dispatch shape
	# DDGIGraph.cpp's own render() actually issued this frame.
	# .z = bits 0-3: DDGIControlFlags bitmask (above), mirrored from
	# DDGIGraph.cpp's Variable<bool>s -- see each flag's own comment there
	# for what it does and why; just the OR of whichever toggles are on
	# (default: all off). Bits 4-5: DDGIOcclusionMode (above), mirrored from
	# DDGIGraph.cpp's Variable<DDGIOcclusionMode> "Occlusion test" -- extract
	# via `(flags.z >> 4) & 0x3`.
	# .w = eviction_grace_frames, mirrored from DDGIGraph.cpp's Variable<int>
	# "Eviction grace (frames)" -- not a 0/1 flag like the others, a small
	# integer. A resident probe that goes unhit rides out up to this many
	# CONSECUTIVE missed frames (DDGI_ProbeMissStreak, DDGIProbeSelect) before
	# DDGIProbeResidencyMark actually evicts it, instead of dropping the
	# instant one frame's pending bit comes back 0 -- a single frame's worth
	# of screen-ray noise/occlusion flicker was observed causing probes to
	# visibly flicker in and out of residency at 0 grace.
	uint4 flags;
}

# Per-probe GPU-resident state. offset is the relocate-out-of-geometry
# adjustment (v1: always zero -- relocation is deferred, see
# DDGIProbeSelect's own comment). flags carries classification bits (v1:
# only a "disabled, too many backfaces on last trace" bit is planned, not
# yet written by anything). last_full_update_frame is what
# DDGIProbeSelect's round-robin reads/writes to decide which probes are due.
struct DDGIProbeMetadata
{
	float3 offset;
	uint flags;
	uint last_full_update_frame;
}

struct DDGIProbes
{
	# Duplicated from DDGIInfo::probe_counts (not read through it) -- a
	# nested [Bind]-struct field can't reach a sibling field of its parent,
	# same reasoning VoxelTilingParams' own voxels_per_tile duplication
	# follows (voxel.sig). Also works around a SIGParser codegen bug: a
	# nested struct whose only field is a StructuredBuffer (no plain field
	# ahead of it) silently drops that field from the generated HLSL table
	# entirely -- confirmed by removing this field and finding `probes`
	# absent from the generated struct with no error. See
	# [[project_jinja2cpp_issues]] for the pattern of known defects in these
	# templates.
	uint4 probe_counts;
	RWStructuredBuffer<DDGIProbeMetadata> probes;

	%{

	uint3 ddgi_probe_grid_coord(uint linear_index, uint3 probe_counts)
	{
		uint3 coord;
		coord.x = linear_index % probe_counts.x;
		coord.y = (linear_index / probe_counts.x) % probe_counts.y;
		coord.z = linear_index / (probe_counts.x * probe_counts.y);
		return coord;
	}

	uint ddgi_probe_linear_index(uint3 probe_grid_coord, uint3 probe_counts)
	{
		return probe_grid_coord.x + probe_grid_coord.y * probe_counts.x + probe_grid_coord.z * probe_counts.x * probe_counts.y;
	}

	// Non-negative modulo (HLSL/C++ % can return negative results for a
	// negative dividend) -- the building block every wrap/toroidal lookup
	// below needs, since a probe's cell coordinate relative to some origin
	// is routinely negative (a corner one cell below the window's own
	// origin, or a slot minus the window's origin when the slot is "before"
	// it in wrapped space).
	int3 ddgi_wrap(int3 v, uint3 counts)
	{
		int3 c = int3(counts);
		return ((v % c) + c) % c;
	}

	// Toroidal (ring-buffer) addressing (see [[project-ddgi]] planning
	// notes): the grid recenters on the camera every frame by snapping
	// grid_min to the nearest whole probe-spacing step (ddgi_make_info,
	// DDGIGraph.cpp), but a probe's ATLAS SLOT (its grid_coord within
	// [0, probe_counts)) is a fixed, unchanging function of its own
	// absolute world-cell position -- NOT of grid_min. Two world cells
	// exactly probe_counts apart alias to the same slot, which is safe
	// precisely because grid_min/probe_counts/spacing together bound the
	// cascade's current working volume to one such cell per residue class
	// (ddgi_sample_irradiance_cascaded's own margin check already enforces
	// this precondition, unchanged by this file).
	//
	// Given grid_min is always an exact multiple of spacing (by
	// construction), window_origin = round(grid_min / spacing) is the
	// window's own minimum corner in integer probe-cell units -- the one
	// piece of state every wrap/unwrap direction below is built from.
	int3 ddgi_window_origin(float3 grid_min, float3 spacing)
	{
		return int3(round(grid_min / spacing));
	}

	// slot -> world position. `probe_grid_coord` is a bare atlas slot with
	// no known relation to any particular world cell (DDGIProbeTrace's own
	// dispatch-derived slot, or an 8-corner sample offset already wrapped
	// into a valid slot by the caller) -- recovers the UNIQUE absolute
	// world-cell in [window_origin, window_origin+probe_counts) that
	// currently maps to it. Reduces to the old direct `grid_min +
	// coord*spacing` when the grid has never scrolled (window_origin's own
	// wrap of a slot already in range is a no-op), so this is a strict
	// superset of the pre-toroidal behavior, not a special case of it.
	float3 ddgi_probe_world_pos(uint3 probe_grid_coord, float3 grid_min, float3 probe_spacing, float3 probe_offset, uint3 probe_counts)
	{
		int3 window_origin = ddgi_window_origin(grid_min, probe_spacing);
		int3 absolute_cell = window_origin + ddgi_wrap(int3(probe_grid_coord) - window_origin, probe_counts);
		return float3(absolute_cell) * probe_spacing + probe_offset;
	}

	// world position -> atlas slot. Pure function of the world cell alone
	// (no grid_min/window_origin involved) -- this is what makes a probe's
	// slot assignment independent of how far the window has scrolled, the
	// entire point of toroidal addressing. Only valid for a world_pos
	// already known to fall within the cascade's current working volume
	// (ddgi_sample_irradiance_cascaded's own margin check, or a hit point a
	// caller has already cascade-selected) -- outside that volume this
	// still returns SOME slot (wrapping never fails), it just may not be
	// the slot the caller actually meant.
	uint3 ddgi_world_to_slot(float3 world_pos, float3 probe_spacing, uint3 probe_counts)
	{
		int3 absolute_cell = int3(floor(world_pos / probe_spacing));
		return uint3(ddgi_wrap(absolute_cell, probe_counts));
	}

	// Texel-space origin (top-left corner) of a probe's cell within the 2D
	// (x,z) plane of any of the DDGI_Probe*/atlas array textures -- probe_y
	// doesn't participate here at all, it's an array-slice offset instead
	// (ddgi_atlas_array_slice, below). All three atlas textures share this
	// same plane layout, so one helper serves radiance, irradiance and
	// visibility lookups alike.
	uint2 ddgi_atlas_origin(uint3 probe_grid_coord, uint texel_size)
	{
		uint2 origin;
		origin.x = probe_grid_coord.x * texel_size;
		origin.y = probe_grid_coord.z * texel_size;
		return origin;
	}

	// Inverse of ddgi_atlas_origin: which probe (x,z) a given atlas-plane
	// texel belongs to. probe_grid_coord.y is NOT recovered here -- callers
	// that dispatch per-cascade already know their own probe.y directly
	// (the dispatch's own 3rd dimension, or ddgi_atlas_array_slice's
	// inverse below when only an array slice is in hand), so it's passed in
	// rather than re-derived.
	uint3 ddgi_atlas_probe_coord(uint2 atlas_texel, uint texel_size, uint probe_grid_y)
	{
		uint2 cell = atlas_texel / texel_size;
		uint3 coord;
		coord.x = cell.x;
		coord.z = cell.y;
		coord.y = probe_grid_y;
		return coord;
	}

	// Which array slice of the shared, DDGI_AtlasArraySlices-deep atlas
	// array a probe's own (probe_y, cascade) pair lives in -- this cascade's
	// own DDGIInfo::cascade_info.y (precomputed in C++, ddgi_make_info) is
	// its slice range's own start, so this is just that plus the probe's
	// local y. Inverse (slice -> probe_y) is `slice - cascade_slice_offset`.
	uint ddgi_atlas_array_slice(uint probe_grid_y, uint cascade_slice_offset)
	{
		return cascade_slice_offset + probe_grid_y;
	}

	float2 ddgi_atlas_local_uv(uint2 atlas_texel, uint texel_size)
	{
		uint2 local = atlas_texel % texel_size;
		return (float2(local) + 0.5) / float(texel_size) * 2.0 - 1.0;
	}

	}%
}

# Per-frame snapshot of DDGI's own GUI toggles (DDGIGraph.cpp's own
# Variable<bool>s, mirrored by ddgi_make_info()'s sibling
# ddgi_update_selectors() -- no owning DDGI instance exists yet, so these
# are free-standing Meyer's-singleton Variables, same pattern
# GBufferDownsampler's own g_roughness_threshold/g_metallic_threshold use,
# VoxelGIGraph.cpp) -- same reasoning as VoxelGISelectors (voxel.sig): the
# generated setups below are static functions with no instance to reach.
struct DDGISelectors
{
	bool enabled = true;
	# Debug visualization toggle (DDGIDebug, below) -- screen-space splat of
	# a marker at each probe's projected position, manually depth-tested
	# against GBuffer_Depth (not a real GraphicsPSO/instanced mesh draw, see
	# [[project-ddgi]] planning notes for why).
	bool show_probes = false;
}

[Bind = DefaultLayout::Instance1]
struct DDGIProbeSelectData
{
	DDGIInfo info;
	DDGIProbes probes;
}

ComputePSO DDGIProbeSelect
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = ddgi/ddgi_probe_select;
}

[Bind = DefaultLayout::Instance0]
struct DDGIProbeTraceData
{
	DDGIInfo info;
	DDGIProbes probes;

	# xyz = shaded radiance, w = hit distance, one entry per traced ray (NOT
	# per atlas texel any more) -- see this PassNode's own comment on
	# DDGI_ProbeRayRadiance for the flat indexing and why there's no texel to
	# write into directly.
	RWStructuredBuffer<float4> probe_ray_radiance;

	# Last frame's convolved output -- see this PassNode's own doc comment
	# above for why reading them here is safe. Sampled at each hit point for
	# the multi-bounce feedback term (ddgi_sample.hlsl).
	Texture2DArray<float4> prev_irradiance;
	Texture2DArray<float2> prev_visibility;

	# This cascade's own residency flags -- read at the top of
	# ddgi_probe_trace.hlsl as a belt-and-suspenders early-out (and again by
	# its own multi-bounce feedback sample, ddgi_sample_irradiance). Redundant
	# for the early-out when g_ddgi_use_indirect_dispatch is on (every probe
	# reached via compacted_list below was already marked needed to get
	# there), but still the only thing skipping the TraceRay call when that
	# toggle is off and this pass still launches the full fixed-size
	# dispatch. StructuredBuffer, not RW -- this pass never writes it (a
	# mismatch against this same buffer's RW declaration as an actual
	# UnorderedAccess resource elsewhere is fine, HLSL just needs the
	# parameter type it's actually used as to match at each call site).
	StructuredBuffer<uint> probe_residency;

	# Maps a compacted (indirect-dispatch) linear index back to which probe
	# it belongs to -- see ddgi_probe_trace.hlsl's own comment. Unused when
	# g_ddgi_use_indirect_dispatch is off (DispatchRaysIndex().xy addresses
	# the atlas directly in that mode, same as before this existed).
	StructuredBuffer<uint> compacted_list;

	# Dilation target -- see the PassNode's own comment (below) on
	# DDGI_ProbeResidencyPending.
	RWStructuredBuffer<uint> residency_pending;
}

[Bind = DefaultLayout::Instance0]
struct DDGIProbeConvolveData
{
	# DDGIInfo only, not DDGIProbes -- convolution needs atlas_info.x
	# (texel size) and probe_counts.x to know which probe/direction a texel
	# belongs to, but never touches the actual probe buffer, so the shader
	# constructs a throwaway DDGIProbes value locally just to call its pure
	# coordinate-math helpers (ddgi_atlas_probe_coord/ddgi_atlas_local_uv,
	# neither of which reads `this`) rather than plumbing DDGI_Probes into
	# this pass at all.
	DDGIInfo info;

	# xyz = shaded radiance, w = hit distance (feeds the visibility mean/
	# mean-square below), one entry per traced ray -- see DDGI_ProbeRayRadiance's
	# own comment (PassNode DDGIProbeTrace) for the flat indexing.
	StructuredBuffer<float4> probe_ray_radiance;
	RWTexture2DArray<float4> probe_irradiance;
	RWTexture2DArray<float2> probe_visibility;

	# See DDGIProbeTraceData's own comment on the same field -- same
	# early-out reasoning (and same StructuredBuffer-not-RW note), applied to
	# the convolution loop instead of a TraceRay call.
	StructuredBuffer<uint> probe_residency;
}

ComputePSO DDGIProbeConvolve
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = ddgi/ddgi_probe_convolve;
}

# Budgeted probe selection -- v1 is a naive round-robin over
# DDGI_MaxProbesPerFrame probes/frame (DDGI.ixx's Variable<int>
# probes_per_frame_budget), not AC Shadows' 3-step greedy priority scheme
# (new/boundary/round-robin-remainder) -- deferred, see plan. Writes
# last_full_update_frame for whichever probes this frame picks;
# DDGIProbeTrace below reads the same buffer to know which ones are due.
#
# [Multiple = 5]: one instance per cascade level (DDGI_CascadeCount), same
# mechanism PSSM_Cascade (pssm.sig) uses for shadow cascades -- NOT [Static]
# because [Multiple] needs runtime-wired render_funcs[N] (DDGI.ixx's
# ddgi_register_passes, PSSM.ixx's own template ctor is the reference), not
# a single generated PassDefault<T>::render. Each instance's data.pass_index
# is its cascade index (0 = finest/smallest spacing).
#
# Also the sole [Size]/[Format] creator of DDGI_ProbeIrradiance/
# DDGI_ProbeVisibility, even though this pass's own shader never touches
# them -- DDGIProbeTrace (below) needs to read them and DDGIProbeConvolve
# (further below) needs to write them, and setup() resolves need()/create()
# calls strictly in this frame's pipeline listing order (test.sig), not by a
# data-flow topological sort. Since DDGIProbeTrace runs BEFORE
# DDGIProbeConvolve, having Convolve be the creator made DDGIProbeTrace's own
# need() assert-fail (ASSERT(exists(result)), FrameGraph.Base.ixx) --
# confirmed the hard way. Creating them here instead (first pass in this
# sub-pipeline, every frame) means both later passes just need()/write()
# a resource that's already guaranteed to exist by the time their own setup
# runs. Same pattern sky_cubemap_filtered uses across this file's own
# consumers (CubeMapEnviromentProcessor is the sole creator; IndirectRTX/
# ReflectionRTX/this file's own DDGIProbeTrace all just [Always=Read] it,
# no [Size]/[Format] of their own).
[Multiple = 5]
[Compute]
[RunAlways]
[SetupCondition = DDGISelectors::enabled]
PassNode DDGIProbeSelect
{
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<DDGIProbeMetadata> DDGI_Probes;

	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight)`] [ArrayCount = `Constants::DDGI_AtlasArraySlices`] [Format = R16G16B16A16_FLOAT] [Optional = data.pass_index == 0]
	Texture DDGI_ProbeIrradiance;
	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight)`] [ArrayCount = `Constants::DDGI_AtlasArraySlices`] [Format = R16G16_FLOAT] [Optional = data.pass_index == 0]
	Texture DDGI_ProbeVisibility;

	# Per-probe-per-cascade residency flag (0/1) -- see
	# DDGIProbeResidencyMark's own PassNode comment below for what sets it
	# and why. Same sole-creator reasoning as Irradiance/Visibility above.
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<uint> DDGI_ProbeResidency;

	# One DispatchRaysArguments (raytracing.sig) record per cascade -- the
	# GPU-driven indirect-DispatchRays args buffer DDGIProbeDispatchArgsBuild
	# (below) fills and DDGIProbeTrace's own ExecuteIndirect reads. Same
	# sole-creator reasoning as the other shared buffers above.
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<DispatchRaysArguments> DDGI_DispatchRaysArgs;

	# Stream-compacted list of this cascade's needed probes (dense, from index
	# 0) plus a per-cascade needed-count -- DDGIProbeResidencyMark (below)
	# appends into it (same pass that sets DDGI_ProbeResidency), and
	# DDGIProbeDispatchArgsBuild/DDGIProbeTrace read the count/list to launch
	# exactly as many rays as there are needed probes instead of the full
	# fixed atlas size. Worst case (every probe needed) is the same size as
	# DDGI_ProbeResidency -- same sole-creator reasoning as the buffers above.
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<uint> DDGI_CompactedProbeList;
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<uint> DDGI_CompactedProbeCount;

	# Hit-point-driven marking's one-frame-lagged inbox (see
	# [[project-ddgi]] planning notes and DDGIProbeResidencyMark's own
	# comment): TraceIndirectDiffuse (IndirectRTX/IndirectRTXHalf,
	# raytracing.hlsl) writes 1s here at its own per-pixel indirect ray's hit
	# point, every frame, for whichever probe cell contains it. Nothing
	# resets this buffer -- DDGIProbeResidencyMark both consumes it (copies
	# into DDGI_ProbeResidency) AND clears it back to 0 in the same pass, so
	# a mark survives from the frame IndirectRTX writes it to the very next
	# frame's residency compaction, then is gone -- exactly the "next frame
	# it will be loaded" propagation this system relies on instead of a
	# same-frame chicken-and-egg (IndirectRTX itself runs AFTER this frame's
	# DDGIProbeResidencyMark/Trace/Convolve, so its own hits can only ever
	# affect NEXT frame's residency, never this one's).
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<uint> DDGI_ProbeResidencyPending;

	# Consecutive frames a currently-resident probe has gone unhit -- lets
	# DDGIProbeResidencyMark ride out a short gap (missed by one frame's
	# worth of screen-ray noise/occlusion flicker, not actually gone) instead
	# of evicting the instant a single frame's pending bit comes back 0. See
	# DDGIInfo::flags.w (eviction_grace_frames) and DDGIProbeResidencyMark's
	# own comment for the exact rule.
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<uint> DDGI_ProbeMissStreak;
}

[Bind = DefaultLayout::Instance0]
struct DDGIProbeResidencyMarkData
{
	DDGIInfo info;
	RWStructuredBuffer<uint> probe_residency;
	RWStructuredBuffer<uint> compacted_list;
	RWStructuredBuffer<uint> compacted_count;
	RWStructuredBuffer<uint> pending;
	RWStructuredBuffer<uint> miss_streak;
	# Selects which of this shader's two jobs to run this dispatch -- see
	# ddgi_probe_residency_mark.hlsl's own comment for why they can't be one
	# dispatch.
	uint reset_only;

	# Toroidal-scroll forced eviction (see [[project-ddgi]] planning notes
	# and ddgi_probe_world_pos/ddgi_world_to_slot's own comments, above):
	# computed once per cascade per frame in ddgi_probe_select_render
	# (DDGIGraph.cpp, which runs first in the pipeline for this cascade)
	# from the delta between this frame's and last frame's window_origin,
	# then handed to this pass. A probe's slot is forcibly evicted on axis A
	# this frame iff `ddgi_wrap(slot[A] - scroll_lo[A], counts[A]) <
	# scroll_count[A]` -- see ddgi_probe_residency_mark.hlsl's own comment
	# for the derivation. scroll_count is pre-clamped to probe_counts on the
	# C++ side, so a jump larger than the grid itself (a teleport/cut)
	# naturally forces every slot on that axis to evict, rather than needing
	# a separate "full reset" code path.
	int3 scroll_lo;
	uint3 scroll_count;
	# Zeroed for an evicted probe's own texel_size^2 cells (see this pass's
	# own comment) so DDGIProbeTrace's next feedback read and
	# DDGIProbeConvolve's next temporal blend see a clean history instead of
	# the previous tenant's unrelated light -- the same "ramp in from zero"
	# a never-before-resident probe already gets, not a blend toward it.
	RWTexture2DArray<float4> probe_irradiance;
	RWTexture2DArray<float2> probe_visibility;
}

ComputePSO DDGIProbeResidencyMark
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = ddgi/ddgi_probe_residency_mark;
}

# Marks which probes (this cascade's own DDGI_ProbeResidency slice) are
# actually needed this frame -- see [[project-ddgi]] planning notes. Real
# marking now: consumes DDGI_ProbeResidencyPending (whatever TraceIndirectDiffuse
# wrote at its own per-pixel indirect ray's hit points LAST frame -- see that
# buffer's own comment, above, for why one frame lagged rather than
# same-frame) into DDGI_ProbeResidency, clearing pending back to 0 as it
# goes, except the coarsest cascade (DDGIInfo::cascade_info.w), which is
# forced fully resident regardless -- it has nowhere further to fall back to
# (cross-cascade fallback for a probe that isn't resident is a later step,
# still not yet implemented -- see ddgi_sample_irradiance_cascaded's own
# comment, ddgi_sample.hlsl). Dilation IS implemented (a probe several hops
# back in the multi-bounce feedback chain marking its own 8 feedback-read
# neighbors needed too, so it doesn't drop the frame it stops being directly
# hit) -- handled at the mark sites themselves (TraceIndirectDiffuse/
# TraceReflection/DDGIProbeTrace's own feedback read), not here; this pass
# only consumes what they wrote. Also toroidal-scroll forced eviction
# (scroll_lo/scroll_count, DDGIProbeResidencyMarkData below) -- see that
# struct's own comment. ALSO stream-compacts the (now real) marked set
# into DDGI_CompactedProbeList/DDGI_CompactedProbeCount (see DDGIProbeSelect's
# own comment on those). Renders TWO dispatches: the first zeroes this
# cascade's own counter slot (can't be folded into the marking dispatch
# itself -- InterlockedAdd from 2048 threads across many groups has no safe
# way to guarantee a zeroing write from one thread happens before another
# thread's add without a separate pass/dispatch boundary). [Multiple=5]: one
# instance per cascade, same mechanism DDGIProbeSelect/Trace/Convolve already
# use (see DDGIProbeSelect's own comment for why [Multiple], not [Static]).
[Multiple = 5]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIProbeResidencyMark
{
	[Always = UnorderedAccess] StructuredBuffer<uint> DDGI_ProbeResidency;
	[Always = UnorderedAccess] StructuredBuffer<uint> DDGI_CompactedProbeList;
	[Always = UnorderedAccess] StructuredBuffer<uint> DDGI_CompactedProbeCount;
	[Always = UnorderedAccess] StructuredBuffer<uint> DDGI_ProbeResidencyPending;
	[Always = UnorderedAccess] StructuredBuffer<uint> DDGI_ProbeMissStreak;
	# Zeroed for a toroidal-scroll-evicted probe's own texels -- see
	# DDGIProbeResidencyMarkData's own comment (this file).
	[Always = UnorderedAccess] Texture DDGI_ProbeIrradiance;
	[Always = UnorderedAccess] Texture DDGI_ProbeVisibility;
}

# Packs this cascade's DDGI_DispatchRaysArgs record for DDGIProbeTrace's
# ExecuteIndirect (DispatchRaysArgsBuild PSO, raytracing.sig). Width is now
# read from DDGI_CompactedProbeCount (DDGIProbeResidencyMark, above) instead
# of the fixed atlas size -- a genuine GPU-computed dispatch size, though
# still numerically identical to the old fixed size today since residency
# marking itself is still the "mark everything" placeholder (see that
# PassNode's own comment). [Multiple=5]: same per-cascade mechanism as
# DDGIProbeSelect/ResidencyMark/Trace/Convolve.
[Multiple = 5]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIProbeDispatchArgsBuild
{
	[Always = Read] StructuredBuffer<uint> DDGI_CompactedProbeCount;
	[Always = UnorderedAccess] StructuredBuffer<DispatchRaysArguments> DDGI_DispatchRaysArgs;
}

# Traces DDGIInfo::rays_per_probe.x (= DDGI_ProbeRayCount) rays per selected
# probe over the sphere (ddgi_sphere_fibonacci, octahedral.hlsl -- no
# precomputed direction table needed), shading each hit with the existing
# direct-sun material path (MyClosestHitShader, universal_material_raytracing.hlsl),
# PLUS the probe-irradiance feedback term: DDGI_ProbeIrradiance/
# DDGI_ProbeVisibility are read here even though DDGIProbeConvolve (below) is
# the one that writes them -- safe because DDGIProbeSelect (above) is their
# sole creator (see its own comment for why) and runs before both. What Trace
# reads here is always LAST frame's convolved result (Convolve hasn't run yet
# THIS frame), never a same-frame value -- exactly the temporal feedback that
# produces multi-bounce GI (see ddgi_probe_trace.hlsl's own doc comment).
#
# Each traced ray's fully-shaded result (direct + feedback) lands in
# DDGI_ProbeRayRadiance -- a flat per-ray buffer, NOT an atlas texture: ray
# count is independent of (and, at 32 vs. the atlas's own 8x8=64 texel
# budget, smaller than) the octahedral output resolution, so there's no 1:1
# texel to write into any more. DDGIProbeConvolve (below) resamples this
# fixed ray set into the actual texel_size^2 output map.
[Multiple = 5]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIProbeTrace
{
	# Read-only dependency on PreScene so the RTX BVH is built/updated before
	# tracing -- same pattern RTXColorPass (raytracing.sig) uses.
	[Always = Read] StructuredBuffer<uint> scene;
	# Force the sky chain to run first so FrameInfo.GetSky() is populated for
	# the miss shader -- same pattern ReflectionRTXHalf/IndirectRTXHalf use
	# (voxel.sig).
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = Read] StructuredBuffer<DDGIProbeMetadata> DDGI_Probes;
	[Always = Read] Texture DDGI_ProbeIrradiance;
	[Always = Read] Texture DDGI_ProbeVisibility;
	[Always = Read] StructuredBuffer<uint> DDGI_ProbeResidency;
	# Not bound to the raygen shader -- read directly as a raw HAL::Resource
	# by this pass's own render() for ExecuteIndirect (DispatchRaysArgsBuild,
	# raytracing.sig, populates it earlier this frame). Declared here purely
	# so FrameGraph orders DDGIProbeDispatchArgsBuild before this pass and
	# tracks the buffer's UAV-write -> indirect-arg-read hazard.
	[Always = Read] StructuredBuffer<DispatchRaysArguments> DDGI_DispatchRaysArgs;
	# The compacted dispatch launches a flat 1D grid over (needed probes x
	# texels/probe) rather than the full atlas -- this is what the raygen
	# shader looks a needed probe's grid coord up from, given only a linear
	# dispatch index (see ddgi_probe_trace.hlsl's own comment).
	[Always = Read] StructuredBuffer<uint> DDGI_CompactedProbeList;
	# Dilation: marks the 8 corner probes this probe's OWN multi-bounce
	# feedback sample reads from as needed too (see ddgi_probe_trace.hlsl's
	# own comment) -- otherwise a probe that's ONLY ever read as a feedback
	# source, never itself hit by a screen ray, has nothing keeping it
	# resident and silently drops out from under whatever's still reading it.
	[Always = UnorderedAccess] StructuredBuffer<uint> DDGI_ProbeResidencyPending;

	# xyz = fully-shaded radiance for this ray (direct + multi-bounce
	# feedback), w = hit distance (feeds DDGIProbeConvolve's visibility mean/
	# mean-square, same reason the old per-texel DDGI_ProbeGBuffer packed hit
	# distance into its own .w instead of a separate buffer). Indexed by
	# (this cascade's own probe_offset + probe_linear_index) * DDGI_ProbeRayCount
	# + ray_index -- flat, not atlas-shaped, since a ray has no texel of its
	# own any more (see this PassNode's own comment for why). Sole creator,
	# same [Optional = pass_index == 0] pattern the old radiance/gbuffer
	# textures used.
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount * Constants::DDGI_ProbeRayCount`] [Optional = data.pass_index == 0]
	StructuredBuffer<float4> DDGI_ProbeRayRadiance;
}

# Cosine-weight-resamples DDGI_ProbeRayRadiance's fixed DDGI_ProbeRayCount
# directions per probe into DDGI_ProbeIrradiance -- the texture
# IndirectRTX/ReflectionRTX's own raygens (raytracing.hlsl) and
# DDGIProbeTrace's own multi-bounce feedback all sample -- and updates
# DDGI_ProbeVisibility (hit-distance moments; the untraced chebyshev/variance
# occlusion test that used to read it was replaced by DDGIOcclusionMode's
# ProbeDepthTest/RTXRay modes, ddgi_sample.hlsl). Each of the
# DDGI_ProbeTexelSize^2 (64) output texels blends ALL DDGI_ProbeRayCount (32)
# rays weighted by cos(angle) between that texel's own octahedral direction
# and each ray's ddgi_sphere_fibonacci direction -- a resample, not the old
# 1:1 read, since there are fewer traced rays than output texels now. LDS-
# prefiltered (ddgi_probe_convolve.hlsl's own comment): one input load per
# probe's own ray set, shared across all 64 output threads via groupshared
# memory, not a naive per-output-texel re-fetch.
[Multiple = 5]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIProbeConvolve
{
	[Always = Read] StructuredBuffer<float4> DDGI_ProbeRayRadiance;
	[Always = Read] StructuredBuffer<uint> DDGI_ProbeResidency;

	# No [Size]/[Format]/[Static] here -- DDGIProbeSelect is the sole creator
	# (see its own comment for why). Plain [Always=UnorderedAccess] just
	# means "this pass writes it", same pattern IndirectRTX/ReflectionRTX use
	# for sky_cubemap_filtered's own read side.
	[Always = UnorderedAccess] Texture DDGI_ProbeIrradiance;
	[Always = UnorderedAccess] Texture DDGI_ProbeVisibility;
}

[Bind = DefaultLayout::Instance0]
struct DDGIDebugData
{
	# One DDGIInfo per cascade -- see DDGIIndirectDebugData's own comment for
	# why named fields, not an array. Every cascade shares the SAME
	# probe_counts (only spacing/atlas offsets differ), so the shader picks
	# one of these 5 by dividing its linear dispatch index by the (shared)
	# per-cascade probe count.
	DDGIInfo cascade0;
	DDGIInfo cascade1;
	DDGIInfo cascade2;
	DDGIInfo cascade3;
	DDGIInfo cascade4;
	DDGIProbes probes;
	Texture2D<float> depth;
	# Sampled toward the camera at each probe's own position, so the marker
	# shows that probe's irradiance on its viewer-facing side -- a rough but
	# intuitive "is this probe actually receiving light" check.
	Texture2DArray<float4> probe_irradiance;
	# Drawn solid red instead of its irradiance color when 0 -- see
	# [[project-ddgi]] planning notes and DDGIProbeResidencyMark's own
	# comment (this file) for what marks/clears this.
	StructuredBuffer<uint> probe_residency;
	RWTexture2D<float4> target;
}

ComputePSO DDGIDebug
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = ddgi/ddgi_debug;
}

# Debug-only screen-space probe visualization (see DDGISelectors::show_probes
# and [[project-ddgi]] planning notes) -- NOT a real GraphicsPSO/instanced
# mesh draw: one thread per probe projects that probe's world position
# through the camera and splats a small block directly into ResultTexture,
# colored by that probe's own convolved irradiance (sampled toward the
# camera, see DDGIDebugData's own comment), with a manual (shader-side)
# depth comparison against GBuffer_DepthMips standing in for real hardware
# depth testing. Placed after the NRD/RTXCombine/ReflCombine chain in
# test.sig's MainPipeline so ResultTexture already holds the fully
# composited lit scene, and before Sky/SMAA/FSR so post-effects don't
# resample a slightly-stale image.
[Static]
[Compute]
[SetupCondition = DDGISelectors::enabled && DDGISelectors::show_probes]
PassNode DDGIDebug
{
	[Always = Read] Texture GBuffer_DepthMips;
	[Always = Read] StructuredBuffer<DDGIProbeMetadata> DDGI_Probes;
	[Always = Read] Texture DDGI_ProbeIrradiance;
	[Always = Read] StructuredBuffer<uint> DDGI_ProbeResidency;
	[Always = UnorderedAccess] Texture ResultTexture;
}

[Bind = DefaultLayout::Instance0]
struct DDGIIndirectDebugData
{
	# One DDGIInfo per cascade (not an array -- fixed-size arrays of a
	# nested [Bind] struct aren't an established pattern in this codebase;
	# named fields mirror Camera/prevCamera's own two-instance precedent,
	# FrameData.sig, just extended to 5). cascade0 is finest/smallest
	# spacing; ddgi_sample_irradiance_cascaded (ddgi_sample.hlsl) tries them
	# in that order and uses the first one whose grid actually contains the
	# shading point.
	DDGIInfo cascade0;
	DDGIInfo cascade1;
	DDGIInfo cascade2;
	DDGIInfo cascade3;
	DDGIInfo cascade4;
	Texture2D<float> depth;
	Texture2D<float4> normals;
	Texture2DArray<float4> probe_irradiance;
	Texture2DArray<float2> probe_visibility;
	# Gates ddgi_sample_irradiance's trilinear blend -- see
	# VoxelOutput::ddgi_residency's own comment (voxel.sig).
	StructuredBuffer<uint> probe_residency;
	# Written at this pass's own per-pixel hit point -- see the PassNode's
	# own comment (below) on why this view marks residency itself instead of
	# only ever reading whatever IndirectRTX marked.
	RWStructuredBuffer<uint> residency_pending;
	RWTexture2D<float4> target;
}

ComputePSO DDGIIndirectDebug
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = ddgi/ddgi_indirect_debug;
}

# Full-screen debug view (selected via FrameGraph::DebugMode::DDGIIndirect,
# see Base.cpp's debug_source()) of the exact same
# ddgi_sample_irradiance() call IndirectRTX's own per-pixel feedback term
# uses (ddgi_sample.hlsl) -- but shown in isolation, at every screen pixel,
# using each pixel's real GBuffer_Normals (not DDGIDebug's marker-only
# camera-facing hack), to judge the trilinear/chebyshev interpolation's
# quality directly instead of only at discrete probe positions. Raw
# irradiance, not albedo-modulated -- same convention as the
# RTXIndirectDenoised debug mode showing the raw GI signal.
[Static]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIIndirectDebug
{
	[Always = Read] Texture GBuffer_Normals;
	[Always = Read] Texture GBuffer_DepthMips;
	[Always = Read] Texture DDGI_ProbeIrradiance;
	[Always = Read] Texture DDGI_ProbeVisibility;
	[Always = Read] StructuredBuffer<uint> DDGI_ProbeResidency;
	# Read-only dependency on PreScene so the RTX BVH is built/updated before
	# this pass's own inline-ray-traced occlusion test (ddgi_sample.hlsl's
	# ddgi_sample_irradiance_cascaded_inline_traced) -- same pattern
	# DDGIProbeTrace's own `scene` field uses.
	[Always = Read] StructuredBuffer<uint> scene;
	# Marks residency at each pixel's own hit point, same as TraceIndirectDiffuse's
	# own marking block (raytracing.hlsl) -- lets this debug view stay
	# accurate under real residency culling (leaving it ON, at full
	# performance) instead of needing "Enable residency culling" turned off
	# just to inspect a given area, which traces/convolves every probe in
	# the whole grid regardless of whether this view is even looking at it.
	#
	# [SkipEnablement]: DDGI_ProbeResidencyPending is Static, and the cull
	# enables every writer of a Static resource regardless of pass order -- so
	# without this, writing it here would keep this debug pass (and everything
	# it reads) alive every frame, even when nothing displays DDGIIndirectDebug.
	# Single bracket, not two -- a second separate [...] group on the same
	# field silently lost its options to a SIGParser codegen bug (traced via
	# the FrameGraph debugger's enablement-chain view: this pass was showing
	# as enabled by its own write to this buffer, meaning SkipEnablement
	# never made it into the generated builder.need() call at all).
	[Always = UnorderedAccess, SkipEnablement] StructuredBuffer<uint> DDGI_ProbeResidencyPending;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture DDGIIndirectDebug;
}
