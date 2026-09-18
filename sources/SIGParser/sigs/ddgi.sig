# DDGI probe-volume GI (see [[project-ddgi]] planning notes) -- a budgeted,
# camera-following grid of world-space probes caching indirect irradiance,
# modeled on Ubisoft's "Raytracing the World of Assassin's Creed Shadows"
# (Advances 2025). Gets multi/infinite-bounce GI by feeding each probe's own
# *previous frame* irradiance back in as incoming light when relighting it
# (talk: "using values from the previous frame ... multi-bounce global
# illumination"), instead of tracing more bounces per pixel per frame --
# IndirectRTX (voxel.sig) still does exactly one ray per pixel.
#
# v1 scaffold: single cascade, fixed grid size, no toroidal recentering yet,
# naive round-robin probe budget instead of AC Shadows' 3-step greedy
# priority scheme, no relocation/classification, no specular probes. All
# deferred refinements are called out at their would-be call site below.

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
# the addressing this implies.
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
# DDGIProbeTrace's raygen dispatches (DDGI.ixx's Variable<int> rays_per_probe);
# the rest of rays_per_probe is padding to a float4-friendly size.
[Bind = DefaultLayout::Instance0]
struct DDGIInfo
{
	float4 grid_min;
	float4 probe_spacing;
	uint4 probe_counts;
	uint4 rays_per_probe;
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

	}%
}

# Per-frame snapshot of DDGI's own GUI toggle (DDGI.ixx's Variable<bool>
# enabled) -- same reasoning as VoxelGISelectors (voxel.sig): the generated
# setups below are static functions with no DDGI instance to reach.
struct DDGISelectors
{
	bool enabled = true;
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
}

[Bind = DefaultLayout::Instance0]
struct DDGIProbeConvolveData
{
	DDGIInfo info;
	DDGIProbes probes;

	Texture2D<float4> probe_radiance;
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
# [Static]/[RunAlways]: self-contained, no DDGI instance state needed beyond
# the mirrored DDGISelectors/DDGIInfo tables.
[Static]
[Compute]
[RunAlways]
[SetupCondition = DDGISelectors::enabled]
PassNode DDGIProbeSelect
{
	[Always = UnorderedAccess | Static] [Size = `(size_t)Constants::DDGI_ProbeCount`]
	StructuredBuffer<DDGIProbeMetadata> DDGI_Probes;
}

# Traces DDGIInfo::rays_per_probe rays per selected probe over the sphere
# (spherical-fibonacci directions -- no precomputed direction table needed),
# shading each hit with the existing direct-sun material path
# (MyClosestHitShader, universal_material_raytracing.hlsl). v1 scaffold: no
# probe-irradiance feedback term yet (that's the multi-bounce step, added
# once DDGIProbeConvolve below is verified producing sane output -- see
# plan's implementation order).
[Static]
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

	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight)`] [Format = R16G16B16A16_FLOAT]
	Texture DDGI_ProbeRadiance;
	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight)`] [Format = R16G16B16A16_FLOAT]
	Texture DDGI_ProbeGBuffer;
}

# Cosine-convolves DDGI_ProbeRadiance into DDGI_ProbeIrradiance -- this is
# the texture the per-pixel indirect-diffuse pass will eventually sample
# (IndirectRTX's raygen, see plan step 5; not wired yet in this scaffold) --
# and updates DDGI_ProbeVisibility (chebyshev depth-test weights for the
# same 8-probe interpolation). v1: naive per-texel convolution loop, not AC
# Shadows' LDS-prefiltered version (deferred, see plan).
[Static]
[Compute]
[SetupCondition = DDGISelectors::enabled && RenderDeviceCapabilities::rtx_supported]
PassNode DDGIProbeConvolve
{
	[Always = Read] Texture DDGI_ProbeRadiance;

	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight)`] [Format = R16G16B16A16_FLOAT]
	Texture DDGI_ProbeIrradiance;
	[Always = UnorderedAccess | Static] [Size = `ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight)`] [Format = R16G16_FLOAT]
	Texture DDGI_ProbeVisibility;
}
