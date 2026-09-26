#include "../autogen/FrameInfo.h"
#include "../autogen/Raytracing.h"
#include "../autogen/tables/RayPayload.h"
#include "../autogen/tables/ShadowPayload.h"
#include "../autogen/tables/ColorShadowPayload.h"
#include "../autogen/rtx/ColorPass.h"
#include "../autogen/DDGIProbeTraceData.h"
#include "octahedral.hlsl"
#include "ddgi_sample.hlsl"
#include "../common/common.hlsl"

// Traces DDGI_ProbeRayCount (info.GetRays_per_probe().x) spherical-fibonacci
// directions per probe -- decoupled from the octahedral output atlas's own
// texel resolution (was 1 ray per radiance-atlas texel, i.e. hard-tied to
// DDGI_ProbeTexelSize^2; see ddgi.prism's DDGI_ProbeRayCount comment for why
// that changed). Each ray's fully-shaded result lands in a flat
// DDGI_ProbeRayRadiance entry, not an atlas texel -- DDGIProbeConvolve
// resamples this fixed ray set into the actual texel grid. Real material
// shading either way: same ColorPass hit group (MyClosestHitShader)
// IndirectRTX's own per-pixel GI ray uses.
//
// Multi-bounce feedback: on a real hit, samples DDGI_ProbeIrradiance/
// DDGI_ProbeVisibility -- LAST frame's convolved output, see
// DDGIProbeTrace's own PassNode comment (ddgi.prism) for why reading them here
// is safe -- at the hit point/normal and adds albedo * irradiance on top of
// the direct-lit result. This is the actual multi/infinite-bounce mechanism
// (AC Shadows talk: "using values from the previous frame ... multi-bounce
// global illumination"): light keeps propagating probe-to-probe over
// successive frames instead of needing more bounces traced per frame. No pi/
// BRDF normalization on the added term yet (tuning item, not structural).
// Retrace skip is real now, just not the AC Shadows per-probe greedy-
// priority scheme the plan called for: residency culling (DDGIProbeResidencyMark)
// skips a probe entirely once nothing marks it needed, and the rotating
// stagger gate (DDGIGraph.cpp's g_ddgi_stagger_k/g_ddgi_stagger_bucket)
// retraces only a rotating fraction of each cascade's probes per frame.
// DDGIProbeSelect itself, the pass meant to own the real per-probe budget,
// is still the original v1 scaffold -- see its own comment.
[shader("raygeneration")]
void DDGIProbeTraceRaygenShader()
{
	// Raygen shaders use CreateX()+a local, not the GetX() global convenience
	// wrapper -- GetX() is guarded out (NO_GLOBAL) for raygen compilation and
	// fails with "undeclared identifier" if used here (confirmed the hard
	// way), same pattern every raygen in raytracing.hlsl follows
	// (CreateVoxelOutput()/CreateVoxelScreen()).
	const DDGIProbeTraceData trace_data = CreateDDGIProbeTraceData();
	const DDGIInfo info = trace_data.GetInfo();
	const DDGIProbes probes = trace_data.GetProbes();
	const Raytracing raytracing = CreateRaytracing();
	const FrameInfo frame = CreateFrameInfo();

	uint ray_count = max(info.GetRays_per_probe().x, 1);

	// Two addressing schemes, matching whichever dispatch shape
	// DDGIGraph.cpp's render() actually issued this frame (info.GetFlags().y,
	// mirrored from g_ddgi_use_indirect_dispatch). Neither touches the
	// octahedral atlas at all any more -- a ray has no texel of its own
	// (ddgi.prism's DDGI_ProbeRayRadiance comment) -- both just need to end up
	// with (probe_coord, ray_index).
	//
	// Fixed-size path: launches a 3D grid of
	// (DDGI_ProbeCountX*DDGI_ProbeRayCount) x DDGI_ProbeCountZ x DDGI_ProbeCountY.
	// Deliberately kept 3D (probe_x/ray folded into the X dimension only, Z
	// and Y kept as real dispatch dimensions) rather than one flat 1D width
	// covering the whole cascade -- see ddgi_probe_trace_render's own
	// comment (DDGIGraph.cpp) for why a single enormous 1D DispatchRays width
	// already caused a real device-removed hang on this hardware/driver.
	//
	// Indirect path (real ExecuteIndirect, see DDGIProbeDispatchArgsBuild/
	// DispatchRaysArgsBuild): launches a flat 1D grid sized to exactly
	// (needed probes x rays/probe) -- safe as flat 1D specifically because
	// residency+stagger compaction keeps its actual width far below the
	// pathological case above.
	uint3 probe_coord;
	uint ray_index;
	if (info.GetFlags().y != 0)
	{
		uint linear_id = DispatchRaysIndex().x;
		uint list_index = linear_id / ray_count;
		ray_index = linear_id % ray_count;

		uint probe_linear_index = trace_data.GetCompacted_list()[info.GetCascade_info().x + list_index];
		probe_coord = probes.ddgi_probe_grid_coord(probe_linear_index, info.GetProbe_counts().xyz);
	}
	else
	{
		uint probe_x = DispatchRaysIndex().x / ray_count;
		ray_index = DispatchRaysIndex().x % ray_count;
		probe_coord = uint3(probe_x, DispatchRaysIndex().z, DispatchRaysIndex().y);
	}

	// Residency early-out (see [[project-ddgi]] planning notes,
	// DDGIProbeResidencyMark's own comment): skip the TraceRay for a probe
	// nothing needs this frame. Redundant in the indirect-dispatch branch
	// above (every entry in DDGI_CompactedProbeList was already marked
	// needed to get there) but still load-bearing in the fixed-dispatch
	// branch, which always launches over the full grid regardless of
	// residency. Leaves the probe's existing ray radiance untouched rather
	// than writing zero, so a probe that stops being needed keeps its last
	// valid value for the cross-cascade fallback / for whenever it's
	// reactivated.
	uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, info.GetProbe_counts().xyz);
	uint probe_buffer_index = info.GetCascade_info().x + probe_linear_index;
	if (trace_data.GetProbe_residency()[probe_buffer_index] == 0)
		return;

	// Stagger early-out (see DDGIGraph.cpp's g_ddgi_stagger_k/
	// g_ddgi_stagger_bucket and DDGIProbeResidencyMark's own identical gate
	// on stream compaction). Redundant in the indirect-dispatch branch above
	// -- every DDGI_CompactedProbeList entry was already both needed AND due
	// to get compacted -- but load-bearing in the fixed-dispatch branch,
	// which, like the residency check just above, always launches over the
	// full grid regardless of whose turn it is this frame.
	uint stagger_k = max(info.GetRays_per_probe().y, 1);
	uint stagger_bucket = info.GetRays_per_probe().z;
	if ((probe_linear_index % stagger_k) != stagger_bucket)
		return;

	float3 dir = ddgi_sphere_fibonacci(ray_index, ray_count);

	// Ray jitter (DDGIInfo::flags.z bit 2, "Jitter probe rays"): without
	// this, a ray always traces the exact same fixed direction every frame
	// it's resident -- if that direction happens to graze past a wall edge
	// or through a small gap into the sky, that leak is a permanent,
	// unchanging bias baked into the probe's stored radiance, not noise that
	// would ever average out (unlike IndirectRTX's own per-pixel rays, which
	// already jitter via blue noise). Nudging the traced direction by a
	// small FRACTION of the inter-ray spacing (not a whole ray-width) means
	// each frame samples a slightly different direction near the same
	// nominal one, so an occasional grazing leak gets diluted by the multi-
	// bounce feedback loop's own frame-to-frame blending instead of
	// persisting exactly. Deliberately small: DDGIProbeConvolve's cosine-
	// weighted resample still treats this ray as if it landed at its exact
	// nominal (unjittered) ddgi_sphere_fibonacci direction, so a large
	// jitter would bias that weighting; a fraction of the spacing keeps the
	// actual sample close enough to nominal for that approximation to hold.
	if ((info.GetFlags().z & (uint)DDGIControlFlags::JitterRays) != 0)
	{
		float2 seed = float2(float(probe_buffer_index), float(ray_index));
		float2 rand = GetRandom2(seed, frame.GetTime());
		// Perturb within the plane tangent to `dir` -- rotating a unit vector
		// by a small offset along two axes orthogonal to it, then
		// renormalizing, same shape as any small-angle direction jitter.
		// Average inter-ray angular spacing on a unit sphere with `ray_count`
		// roughly-uniform points is ~sqrt(4*PI/ray_count); nudging by a
		// fraction of THAT (not a fixed constant) keeps the jitter
		// proportional to however dense/sparse the ray set actually is.
		float3 up = abs(dir.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
		float3 tangent = normalize(cross(up, dir));
		float3 bitangent = cross(dir, tangent);
		float spacing = sqrt(4.0 * PI / float(ray_count));
		float2 jitter = (rand - 0.5) * (0.35 * spacing);
		dir = normalize(dir + tangent * jitter.x + bitangent * jitter.y);
	}

	float3 probe_pos = probes.ddgi_probe_world_pos(probe_coord, info.GetGrid_min().xyz, info.GetProbe_spacing().xyz, float3(0, 0, 0), info.GetProbe_counts().xyz);

	[raypayload] RayPayload payload_gi = CreateRayPayload();
	// Swap the real recursive RTX shadow ray MyClosestHitShader normally
	// fires for a single cheap VSM lookup instead (see RayPayload::
	// use_vsm_shadow's own comment, raytracing.prism) -- DDGI traces far more
	// rays per frame than any other RTX consumer, and a probe's own shadow
	// term doesn't need a primary screen ray's precision.
	payload_gi.use_vsm_shadow = 1;

	RayDesc ray;
	ray.Origin = probe_pos;
	ray.Direction = dir;
	ray.TMin = 0.0001;
	ray.TMax = 10000.0;
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload_gi);

	// Trace-time self-feedback has its own on/off (DDGIInfo::flags.z bit 3,
	// "Use fallback while generating probes"), separate from flags.x (the
	// final per-pixel screen term, TraceIndirectDiffuse) -- turning THIS one
	// off stops a probe's own multi-bounce term from ever reading back into
	// itself/its neighbors while probes are being generated, isolating
	// whether a leak is coming from direct-lit shading alone or being
	// amplified/introduced by the recursive feedback loop across frames.
	bool trace_feedback_disabled = (info.GetFlags().z & (uint)DDGIControlFlags::DisableTraceFeedback) != 0;

	float3 result_color = payload_gi.color.rgb;
	if (payload_gi.dist > 0.0 && info.GetFlags().x != 0 && !trace_feedback_disabled)
	{
		float3 hit_pos = ray.Origin + ray.Direction * payload_gi.dist;

		// Dilation (see [[project-ddgi]] planning notes and
		// DDGI_ProbeResidencyPending's own comment, ddgi.prism): marks the same
		// 8 corner probes the feedback sample right below is about to read as
		// needed too, one cascade at a time (this probe's own -- unlike
		// TraceIndirectDiffuse's marking block, which doesn't know which
		// cascade will end up serving a given point, this call is already
		// scoped to exactly one). Without this, a probe that's ONLY ever
		// read as a feedback source -- never itself hit by a screen ray --
		// has nothing keeping it resident, so it can silently drop out from
		// under whatever's still reading it, which is what made "Enable
		// residency culling" visibly change the result: some of what a
		// sample blended in when culling was off had never been marked in
		// the first place.
		{
			float3 local = (hit_pos - info.GetGrid_min().xyz) / info.GetProbe_spacing().xyz;
			int3 dilate_base = int3(floor(local));
			uint3 dilate_counts = info.GetProbe_counts().xyz;
			uint dilate_offset = info.GetCascade_info().x;
			// Absolute-cell offset for this cascade's current window -- see
			// ddgi_sample_irradiance's own comment (ddgi_sample.hlsl) for why
			// marking must address by ABSOLUTE (wrapped) cell, not the local
			// (window-relative) one: two different local coordinates for the
			// same probe alias to different slots depending on where the
			// window currently sits, which is exactly the bug toroidal
			// addressing exists to avoid.
			int3 dilate_window_origin = probes.ddgi_window_origin(info.GetGrid_min().xyz, info.GetProbe_spacing().xyz);

			// Still bounds-checked (not unconditionally wrapped): hit_pos is
			// an arbitrary traced hit with no margin guarantee against THIS
			// cascade's grid the way a sample point already has (that
			// guarantee comes from ddgi_sample_irradiance_cascaded's own
			// cascade-selection check, which nothing here goes through) --
			// wrapping an out-of-window corner unconditionally would alias
			// it onto a real, unrelated probe on the opposite side of the
			// grid instead of just skipping it.
			[unroll]
			for (uint di = 0; di < 8; di++)
			{
				int3 corner = int3(di & 1, (di >> 1) & 1, (di >> 2) & 1);
				int3 cell = dilate_base + corner;
				if (all(cell >= 0) && all(cell < int3(dilate_counts)))
				{
					uint3 wrapped = uint3(probes.ddgi_wrap(dilate_window_origin + cell, dilate_counts));
					uint dilate_linear_index = probes.ddgi_probe_linear_index(wrapped, dilate_counts);
					trace_data.GetResidency_pending()[dilate_offset + dilate_linear_index] = 1;
				}
			}
		}

		float3 indirect = ddgi_sample_irradiance(hit_pos, payload_gi.hit_normal, info,
			trace_data.GetPrev_irradiance(), trace_data.GetPrev_visibility(), trace_data.GetProbe_residency(),
			raytracing.GetScene());
		// Feedback strength (DDGIInfo::probe_spacing.w, DDGIGraph.cpp's
		// "Feedback strength") -- damps how strongly this probe's own
		// multi-bounce self-feedback loop reinforces itself per hop, on top
		// of the existing albedo attenuation. Does not touch
		// TraceIndirectDiffuse/TraceReflection's own final per-pixel term.
		result_color += payload_gi.albedo * indirect * info.GetProbe_spacing().w;
	}

	// NaN/Inf guard (see ddgi_probe_convolve.hlsl's own comment for the full
	// story): this is the actual entry point for a bad value into the whole
	// system -- the feedback term above reads OTHER probes' stored
	// irradiance, so if one of them is already bad, `indirect` (and hence
	// `result_color`) inherits it here and would otherwise carry it forward
	// into DDGI_ProbeRayRadiance, letting DDGIProbeConvolve read it as if it
	// were legitimate. Catching it at the write means a poisoned neighbor
	// can't use THIS probe to spread further, even before Convolve's own
	// guard heals the neighbor itself.
	if (any(isnan(result_color)) || any(isinf(result_color)))
		result_color = payload_gi.color.rgb;

	uint ray_buffer_index = probe_buffer_index * ray_count + ray_index;
	trace_data.GetProbe_ray_radiance()[ray_buffer_index] = float4(result_color, payload_gi.dist);
}
