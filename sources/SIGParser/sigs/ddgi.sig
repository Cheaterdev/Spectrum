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

# Single-cascade grid size, shared between DDGI.ixx's grid bookkeeping and
# every [Size=...] below -- one source of truth, same reasoning as vsm.sig's
# MaxLevels/VSM_PagesPerLevelSide.
const DDGI_ProbeCountX = 16;
const DDGI_ProbeCountY = 8;
const DDGI_ProbeCountZ = 16;
const DDGI_ProbeCount = `Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeCountY * Constants::DDGI_ProbeCountZ`;

# Octahedral atlas texel budget per probe. v1 uses the same texel size for
# radiance/irradiance/visibility for scaffold simplicity -- AC Shadows uses
# different sizes per buffer (10x10 radiance, 5x5 irradiance, 20x20
# visibility, all with borders) to trade memory for filtering quality; right-
# sizing each independently is a follow-up, not a correctness requirement.
const DDGI_ProbeTexelSize = 8;
# Flattens (probe_x, probe_z) into the atlas' horizontal axis, probe_y into
# the vertical axis -- see DDGIProbes' ddgi_atlas_origin() helper below for
# the addressing this implies. Single-cascade width/height -- the actual
# allocated atlas is DDGI_CascadeCount times wider (see PassNode comments
# above); this constant stays the per-cascade size so ddgi_atlas_origin's
# LOCAL (cascade-relative) math doesn't need to change, only the final
# global-texel offset added at each actual texture read/write.
const DDGI_AtlasWidth  = `Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeCountZ * Constants::DDGI_ProbeTexelSize`;
const DDGI_AtlasHeight = `Constants::DDGI_ProbeCountY * Constants::DDGI_ProbeTexelSize`;

# Per-frame probe-update budget (DDGI.ixx's Variable<int> probes_per_frame_budget
# mirrors into DDGISelectors below); this constant is only the storage-independent
# upper bound used to size nothing in particular yet -- kept for parity with
# vsm.sig's MaxDispatchEntries pattern in case a future selection pass needs a
# GPU-side counted append buffer sized off it.
const DDGI_MaxProbesPerFrame = `Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeCountY`;

# Mirrored once per frame by DDGI::update_frame() (DDGIGraph.cpp), same
# reasoning as VoxelInfo (voxel.sig): grid_min is the world-space corner of
# probe (0,0,0), probe_spacing the world-space distance between adjacent
# probes along each axis. rays_per_probe.x is the actual per-probe ray count
# (informational only in v1 -- DDGIProbeTrace's raygen currently dispatches
# one ray per radiance-atlas texel instead, see its own doc comment); the
# rest of rays_per_probe is padding to a float4-friendly size. atlas_info.x
# is DDGI_ProbeTexelSize -- HLSL has no access to the generated Constants::
# namespace (C++-only), so shaders that need to convert an atlas texel
# coordinate to a probe grid coordinate (DDGIProbes::ddgi_atlas_probe_coord/
# ddgi_atlas_local_uv, below) read it from here instead. cascade_info.x is
# this cascade's linear offset into the (DDGI_CascadeCount-times-larger)
# shared DDGI_Probes buffer; .y is its X-texel offset into the shared
# (DDGI_CascadeCount-times-wider) atlas textures; .z is the cascade index
# itself (informational/debug only). All precomputed in C++
# (DDGIGraph.cpp's ddgi_make_info) since HLSL can't reach Constants:: to
# derive them itself, same reasoning atlas_info.x already documents.
[Bind = DefaultLayout::Instance0]
struct DDGIInfo
{
	float4 grid_min;
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
	# comparison against plain 1-bounce RTX. yzw unused.
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

	// Toroidal wraparound, same formula the AC Shadows talk gives for its own
	// atlas addressing: index = (position/probeSize) % numGridElements. v1's
	// grid never actually moves yet (no recentering, see DDGI.ixx's own
	// comment), so this is currently equivalent to an identity wrap -- kept
	// now so recentering is a pure C++ change later, not a shader rewrite.
	uint3 ddgi_toroidal_wrap(uint3 probe_grid_coord, uint3 probe_counts)
	{
		return probe_grid_coord % probe_counts;
	}

	float3 ddgi_probe_world_pos(uint3 probe_grid_coord, float3 grid_min, float3 probe_spacing, float3 probe_offset)
	{
		return grid_min + float3(probe_grid_coord) * probe_spacing + probe_offset;
	}

	// Texel-space origin (top-left corner) of a probe's cell in any of the
	// DDGI_Probe*/atlas textures -- all three share the same flattened
	// (x,z)-then-y layout (see DDGI_AtlasWidth/Height above), so one helper
	// serves radiance, irradiance and visibility lookups alike.
	uint2 ddgi_atlas_origin(uint3 probe_grid_coord, uint probe_counts_x, uint texel_size)
	{
		uint2 origin;
		origin.x = (probe_grid_coord.x + probe_grid_coord.z * probe_counts_x) * texel_size;
		origin.y = probe_grid_coord.y * texel_size;
		return origin;
	}

	// Inverse of ddgi_atlas_origin: which probe (and which texel-center UV
	// within that probe's octahedral cell, in [-1,1]) a given atlas texel
	// belongs to. Used by DDGIProbeTrace to know which probe to trace from
	// and which direction that texel represents (via octahedral decode of
	// the returned UV, see octahedral.hlsl).
	uint3 ddgi_atlas_probe_coord(uint2 atlas_texel, uint texel_size, uint probe_counts_x)
	{
		uint2 cell = atlas_texel / texel_size;
		uint3 coord;
		coord.x = cell.x % probe_counts_x;
		coord.z = cell.x / probe_counts_x;
		coord.y = cell.y;
		return coord;
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

	RWTexture2D<float4> probe_radiance;
	# Packed (normal.xyz, hit distance) per traced direction -- lets
	# DDGIProbeSelect's future lighting-only refresh (deferred, see plan)
	# relight without retracing, same rationale as AC Shadows' own kept
	# probe G-buffer.
	RWTexture2D<float4> probe_gbuffer;

	# Last frame's convolved output -- see this PassNode's own doc comment
	# above for why reading them here is safe. Sampled at each hit point for
	# the multi-bounce feedback term (ddgi_sample.hlsl).
	Texture2D<float4> prev_irradiance;
	Texture2D<float2> prev_visibility;
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

	Texture2D<float4> probe_radiance;
	# Hit distance (its .w) feeds the chebyshev visibility mean/mean-square
	# below -- same reason DDGIProbeTrace packs it here instead of only in
	# probe_radiance's alpha.
	Texture2D<float4> probe_gbuffer;
	RWTexture2D<float4> probe_irradiance;
	RWTexture2D<float2> probe_visibility;
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

	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth * Constants::DDGI_CascadeCount, Constants::DDGI_AtlasHeight)`] [Format = R16G16B16A16_FLOAT] [Optional = data.pass_index == 0]
	Texture DDGI_ProbeIrradiance;
	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth * Constants::DDGI_CascadeCount, Constants::DDGI_AtlasHeight)`] [Format = R16G16_FLOAT] [Optional = data.pass_index == 0]
	Texture DDGI_ProbeVisibility;
}

# Traces DDGIInfo::rays_per_probe rays per selected probe over the sphere
# (spherical-fibonacci directions -- no precomputed direction table needed),
# shading each hit with the existing direct-sun material path
# (MyClosestHitShader, universal_material_raytracing.hlsl), PLUS the
# probe-irradiance feedback term: DDGI_ProbeIrradiance/DDGI_ProbeVisibility
# are read here even though DDGIProbeConvolve (below) is the one that writes
# them -- safe because DDGIProbeSelect (above) is their sole creator (see its
# own comment for why) and runs before both. What Trace reads here is always
# LAST frame's convolved result (Convolve hasn't run yet THIS frame), never a
# same-frame value -- exactly the temporal feedback that produces
# multi-bounce GI (see ddgi_probe_trace.hlsl's own doc comment).
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

	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth * Constants::DDGI_CascadeCount, Constants::DDGI_AtlasHeight)`] [Format = R16G16B16A16_FLOAT] [Optional = data.pass_index == 0]
	Texture DDGI_ProbeRadiance;
	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth * Constants::DDGI_CascadeCount, Constants::DDGI_AtlasHeight)`] [Format = R16G16B16A16_FLOAT] [Optional = data.pass_index == 0]
	Texture DDGI_ProbeGBuffer;
}

# Cosine-convolves DDGI_ProbeRadiance into DDGI_ProbeIrradiance -- this is
# the texture the per-pixel indirect-diffuse pass will eventually sample
# (IndirectRTX's raygen, see plan step 5; not wired yet in this scaffold) --
# and updates DDGI_ProbeVisibility (chebyshev depth-test weights for the
# same 8-probe interpolation). v1: naive per-texel convolution loop, not AC
# Shadows' LDS-prefiltered version (deferred, see plan).
[Multiple = 5]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIProbeConvolve
{
	[Always = Read] Texture DDGI_ProbeRadiance;
	[Always = Read] Texture DDGI_ProbeGBuffer;

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
	Texture2D<float4> probe_irradiance;
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
	Texture2D<float4> probe_irradiance;
	Texture2D<float2> probe_visibility;
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

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture DDGIIndirectDebug;
}
