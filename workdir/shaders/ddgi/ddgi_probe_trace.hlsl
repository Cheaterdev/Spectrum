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

// v1 (see [[project-ddgi]] planning notes): one ray per radiance-atlas texel
// (dispatch dims = DDGI_AtlasWidth x DDGI_AtlasHeight x DDGI_ProbeCountY in
// the fixed-size path, see below), direction from octahedral-decoding that
// texel's own local UV within its probe's cell.
// Real DDGI implementations decouple ray count from texel resolution (trace
// a small fixed set of directions per probe, then resample into the
// octahedral map) -- this 1:1 mapping is a v1 simplification, not the final
// design, but it does trace and shade for real: same ColorPass hit group
// (MyClosestHitShader) IndirectRTX's own per-pixel GI ray uses, so a probe's
// radiance texel gets genuine direct-sun-lit material shading.
//
// Multi-bounce feedback: on a real hit, samples DDGI_ProbeIrradiance/
// DDGI_ProbeVisibility -- LAST frame's convolved output, see
// DDGIProbeTrace's own PassNode comment (ddgi.sig) for why reading them here
// is safe -- at the hit point/normal and adds albedo * irradiance on top of
// the direct-lit result. This is the actual multi/infinite-bounce mechanism
// (AC Shadows talk: "using values from the previous frame ... multi-bounce
// global illumination"): light keeps propagating probe-to-probe over
// successive frames instead of needing more bounces traced per frame. No pi/
// BRDF normalization on the added term yet (tuning item, not structural) and
// no round-robin budget skip -- every probe retraces every frame for now
// (both deferred, see plan).
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

	uint texel_size = info.GetAtlas_info().x;

	// Two addressing schemes, matching whichever dispatch shape
	// DDGIGraph.cpp's render() actually issued this frame (info.GetFlags().y,
	// mirrored from g_ddgi_use_indirect_dispatch). Either way the atlas is a
	// Texture2DArray: the 2D plane only ever holds (probe_x, probe_z);
	// probe_y and the cascade both live in the array dimension instead
	// (DDGIProbeSelect's own comment, ddgi.sig, on why -- D3D12's 16384 max
	// texture dimension).
	//
	// Fixed-size path: launches a 3D WxHxDDGI_ProbeCountY grid over the full
	// atlas -- xy is DispatchRaysIndex().xy (the plane), z is probe_y
	// directly.
	//
	// Indirect path (real ExecuteIndirect, see DDGIProbeDispatchArgsBuild/
	// DispatchRaysArgsBuild): launches a flat 1D grid sized to exactly
	// (needed probes x texels/probe) -- there is no atlas position to decode
	// directly from a 1D index, so instead: divide out which compacted-list
	// entry (which probe) and which texel within that probe's own cell,
	// look the probe's full grid coord (including its own y) up from
	// DDGI_CompactedProbeList, then reconstruct its atlas-plane origin
	// (ddgi_atlas_origin, the exact inverse of ddgi_atlas_probe_coord below)
	// and add the local texel back on.
	uint2 atlas_texel;
	uint3 probe_coord;
	if (info.GetFlags().y != 0)
	{
		uint texels_per_probe = texel_size * texel_size;
		uint linear_id = DispatchRaysIndex().x;
		uint list_index = linear_id / texels_per_probe;
		uint local_texel_linear = linear_id % texels_per_probe;
		uint2 local_texel = uint2(local_texel_linear % texel_size, local_texel_linear / texel_size);

		uint probe_linear_index = trace_data.GetCompacted_list()[info.GetCascade_info().x + list_index];
		probe_coord = probes.ddgi_probe_grid_coord(probe_linear_index, info.GetProbe_counts().xyz);
		atlas_texel = probes.ddgi_atlas_origin(probe_coord, texel_size) + local_texel;
	}
	else
	{
		atlas_texel = DispatchRaysIndex().xy;
		uint probe_y = DispatchRaysIndex().z;
		probe_coord = probes.ddgi_atlas_probe_coord(atlas_texel, texel_size, probe_y);
	}

	uint slice = probes.ddgi_atlas_array_slice(probe_coord.y, info.GetCascade_info().y);

	// Residency early-out (see [[project-ddgi]] planning notes,
	// DDGIProbeResidencyMark's own comment): skip the TraceRay for a probe
	// nothing needs this frame. Redundant in the indirect-dispatch branch
	// above (every entry in DDGI_CompactedProbeList was already marked
	// needed to get there) but still load-bearing in the fixed-dispatch
	// branch, which always launches over the full atlas regardless of
	// residency. Leaves the probe's existing radiance/gbuffer texels
	// untouched rather than writing zero, so a probe that stops being needed
	// keeps its last valid value for the cross-cascade fallback / for
	// whenever it's reactivated.
	uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, info.GetProbe_counts().xyz);
	uint probe_buffer_index = info.GetCascade_info().x + probe_linear_index;
	if (trace_data.GetProbe_residency()[probe_buffer_index] == 0)
		return;

	float2 local_uv = probes.ddgi_atlas_local_uv(atlas_texel, texel_size);

	// Ray jitter (DDGIInfo::flags.z bit 2, "Jitter probe rays"): without
	// this, a texel always traces the exact same fixed direction every
	// frame it's resident -- if that direction happens to graze past a wall
	// edge or through a small gap into the sky, that leak is a permanent,
	// unchanging bias baked into the probe's stored radiance, not noise
	// that would ever average out (unlike IndirectRTX's own per-pixel rays,
	// which already jitter via blue noise). Nudging the sampled UV by a
	// small FRACTION of a texel (not a whole one) means each frame samples
	// a slightly different direction near the same nominal one, so an
	// occasional grazing leak gets diluted by the multi-bounce feedback
	// loop's own frame-to-frame blending instead of persisting exactly.
	// Deliberately small: DDGIProbeConvolve's cosine-weighted convolution
	// still treats each texel as if it were sampled at its exact nominal
	// (unjittered) direction, so a large jitter would bias that weighting;
	// a fraction of a texel keeps the actual sample close enough to nominal
	// for that approximation to hold.
	if ((info.GetFlags().z & (uint)DDGIControlFlags::JitterRays) != 0)
	{
		float2 seed = float2(atlas_texel) + float2(slice * 97u, slice * 131u);
		float2 rand = GetRandom2(seed, frame.GetTime());
		float2 jitter = (rand - 0.5) * (0.7 / texel_size);
		local_uv = clamp(local_uv + jitter, -1.0, 1.0);
	}

	float3 dir = ddgi_oct_decode(local_uv);

	float3 probe_pos = probes.ddgi_probe_world_pos(probe_coord, info.GetGrid_min().xyz, info.GetProbe_spacing().xyz, float3(0, 0, 0), info.GetProbe_counts().xyz);

	[raypayload] RayPayload payload_gi;
	payload_gi.init();
	// Swap the real recursive RTX shadow ray MyClosestHitShader normally
	// fires for a single cheap VSM lookup instead (see RayPayload::
	// use_vsm_shadow's own comment, raytracing.sig) -- DDGI traces far more
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
		// DDGI_ProbeResidencyPending's own comment, ddgi.sig): marks the same
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
	// into DDGI_ProbeRadiance, letting DDGIProbeConvolve read it as if it
	// were legitimate. Catching it at the write means a poisoned neighbor
	// can't use THIS probe to spread further, even before Convolve's own
	// guard heals the neighbor itself.
	if (any(isnan(result_color)) || any(isinf(result_color)))
		result_color = payload_gi.color.rgb;

	// atlas_texel is already the correct (x,z)-plane position -- slice
	// (computed above) is what lands the write in this probe's own layer of
	// the shared atlas array.
	trace_data.GetProbe_radiance()[uint3(atlas_texel, slice)] = float4(result_color, 1);
	trace_data.GetProbe_gbuffer()[uint3(atlas_texel, slice)] = float4(dir, payload_gi.dist);
}
