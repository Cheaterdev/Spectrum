#include "../autogen/tables/DDGIProbes.h"
#include "../autogen/DDGIProbeConvolveData.h"
#include "octahedral.hlsl"

// Cosine-weighted RESAMPLE of a probe's own fixed traced ray set
// (DDGI_ProbeRayRadiance, ddgi_sphere_fibonacci directions -- see ddgi.sig's
// DDGI_ProbeRayCount comment) into one irradiance texel per output direction,
// plus a mean/mean-square hit-distance visibility texel (depth-test input for
// DDGIOcclusionMode::ProbeDepthTest, ddgi_sample.hlsl). Not a 1:1 read any
// more -- there are fewer traced rays (DDGI_ProbeRayCount, e.g. 32) than
// output texels (DDGI_ProbeTexelSize^2, 64), so every output texel blends the
// SAME fixed ray set, weighted by cos(angle) to each ray's own direction,
// same shape the old per-texel convolution used when ray and texel were 1:1.
//
// LDS-prefiltered (AC Shadows' own approach): one 8x8 threadgroup maps
// exactly onto one probe's own texel_size x texel_size output cell
// (numthreads below is hardcoded to match Constants::DDGI_ProbeTexelSize,
// same fixed coupling the AtlasWidth/AtlasHeight dispatch dimensions already
// assume -- HLSL can't reach the generated Constants:: namespace, same
// reasoning DDGIInfo::atlas_info.x's own comment gives), so every one of the
// 64 threads in a group needs the SAME ray_count-sized input set -- naively,
// that's 64*ray_count reads per probe when only ray_count unique rays exist.
// The first ray_count threads (linearized group-thread-index < ray_count)
// each load one ray into groupshared once, then every thread's resample loop
// below reads from LDS instead of the buffer. Requires ray_count <= 64 (the
// thread-group size) for one-thread-per-ray loading to cover the whole set in
// a single pass -- true for DDGI_ProbeRayCount's current value (32) with
// headroom, but would need a second load per thread if ray_count ever grew
// past the group size.
groupshared float4 gs_ray_radiance[64];

[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID)
{
	const DDGIProbeConvolveData data = GetDDGIProbeConvolveData();
	// Dispatch size is one cascade's own DDGI_AtlasWidth x DDGI_AtlasHeight x
	// DDGI_ProbeCountY -- xy is the (probe_x, probe_z) plane, z is probe_y
	// directly (see DDGIProbeSelect's own comment, ddgi.sig, on why probe_y
	// and the cascade both live in the array dimension instead of a folded
	// 2D width/height). All the octahedral/probe-cell math below stays in
	// plane-local space; only the final texture writes add the array slice
	// (this cascade's own offset, DDGIInfo::cascade_info.y, plus probe_y).
	uint2 local_texel = dispatchID.xy;
	uint probe_y = dispatchID.z;
	uint texel_size = data.GetInfo().GetAtlas_info().x;
	uint ray_count = max(data.GetInfo().GetRays_per_probe().x, 1);

	// No bound buffer -- DDGIProbes here is only ever used for its pure
	// coordinate-math helpers (ddgi_atlas_probe_coord/ddgi_atlas_local_uv,
	// neither reads `this`), see DDGIProbeConvolveData's own comment (ddgi.sig)
	// for why the actual probe buffer isn't plumbed into this pass at all.
	DDGIProbes probes;

	uint3 probe_coord = probes.ddgi_atlas_probe_coord(local_texel, texel_size, probe_y);
	uint slice = probes.ddgi_atlas_array_slice(probe_y, data.GetInfo().GetCascade_info().y);

	// Residency early-out -- see ddgi_probe_trace.hlsl's own comment on the
	// same check. Skips the resample loop entirely for a probe nothing needs
	// this frame, leaving its irradiance/visibility texels at their last
	// valid value.
	uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, data.GetInfo().GetProbe_counts().xyz);
	uint probe_buffer_index = data.GetInfo().GetCascade_info().x + probe_linear_index;
	if (data.GetProbe_residency()[probe_buffer_index] == 0)
		return;

	// Stagger early-out -- same rotating-subset gate DDGIProbeTrace/
	// DDGIProbeResidencyMark use (DDGIGraph.cpp's g_ddgi_stagger_k/
	// g_ddgi_stagger_bucket, mirrored via DDGIInfo::rays_per_probe.yz).
	// DDGIProbeConvolve is always a full fixed-size dispatch (no indirect
	// variant of its own -- see ddgi_probe_convolve_render's own comment), so
	// this is the only place that skip happens for this pass; reconvolving a
	// probe DDGIProbeTrace didn't retrace this frame would just reproduce the
	// irradiance it already holds.
	uint stagger_k = max(data.GetInfo().GetRays_per_probe().y, 1);
	uint stagger_bucket = data.GetInfo().GetRays_per_probe().z;
	if ((probe_linear_index % stagger_k) != stagger_bucket)
		return;

	// Every thread in this group shares the same probe_coord (local_texel
	// only varies within one texel_size-sized cell, which
	// ddgi_atlas_probe_coord's own division collapses to one probe), so the
	// residency/stagger checks above are uniform across the whole group --
	// either all 64 threads return here together, or all 64 proceed
	// together. Safe to load into groupshared and sync below without a
	// partial-group deadlock risk (some threads waiting at the barrier while
	// others already returned).
	uint linear_gtid = gtid.y * 8 + gtid.x;
	// probe_buffer_index * ray_count is DDGIProbeTrace's own ray_buffer_index
	// base (ddgi_probe_trace.hlsl) -- ray_count here is the runtime-mirrored
	// DDGI_ProbeRayCount (DDGIInfo::rays_per_probe.x), same value that sized
	// each probe's own contiguous run in DDGI_ProbeRayRadiance.
	uint ray_buffer_base = probe_buffer_index * ray_count;
	if (linear_gtid < ray_count)
		gs_ray_radiance[linear_gtid] = data.GetProbe_ray_radiance()[ray_buffer_base + linear_gtid];
	GroupMemoryBarrierWithGroupSync();

	float2 out_uv = probes.ddgi_atlas_local_uv(local_texel, texel_size);
	float3 out_dir = ddgi_oct_decode(out_uv);

	float3 radiance_sum = 0;
	float weight_sum = 0;
	float dist_sum = 0;
	float dist2_sum = 0;
	float depth_weight_sum = 0;

	for (uint i = 0; i < ray_count; i++)
	{
		// Recomputed, not stored -- ddgi_sphere_fibonacci is a pure function
		// of (index, count), and DDGIProbeTrace fed it the exact same
		// (i, ray_count) pair when it fired this ray, so this reconstructs
		// the same direction without needing a separate direction buffer.
		float3 ray_dir = ddgi_sphere_fibonacci(i, ray_count);

		float cos_theta = dot(out_dir, ray_dir);
		float weight = max(cos_theta, 0.0);
		if (weight <= 0.0)
			continue;

		float4 ray_radiance = gs_ray_radiance[i];
		radiance_sum += ray_radiance.rgb * weight;
		weight_sum += weight;

		// Distance/visibility needs a much SHARPER, direction-specific
		// kernel than irradiance's wide diffuse lobe -- a hit a few units
		// away in one direction and a sky miss (dist=100000, MyMissShader) a
		// few directions over must NOT blend, or the resulting variance is
		// dominated by the miss outlier and the chebyshev test below
		// (variance/(variance+diff^2)) collapses to ~1.0 (never occluded)
		// regardless of the real occluder -- confirmed the hard way as the
		// actual cause of oversized light leaks, not just the probe/backface
		// weighting. pow(.,64) is the standard DDGI-paper sharpening
		// exponent for this.
		float depth_weight = pow(saturate(cos_theta), 64.0);
		float hit_dist = ray_radiance.w;
		dist_sum  += hit_dist * depth_weight;
		dist2_sum += hit_dist * hit_dist * depth_weight;
		depth_weight_sum += depth_weight;
	}

	float3 irradiance = weight_sum > 0.0 ? radiance_sum / weight_sum : float3(0, 0, 0);
	float mean_dist   = depth_weight_sum > 0.0 ? dist_sum / depth_weight_sum : 0.0;
	float mean_dist2  = depth_weight_sum > 0.0 ? dist2_sum / depth_weight_sum : 0.0;

	// NaN/Inf guard (see [[project-ddgi]] planning notes -- "use fallback
	// while generating probes" was observed poisoning the whole grid black
	// with no way back). A single corrupted value here, once blended into
	// probe_irradiance/probe_visibility below, propagates forever:
	// lerp(NaN, anything, alpha) is NaN regardless of alpha, every later
	// reader (this probe's own next trace, any sample that blends this
	// probe in) inherits it, and DDGIProbeTrace's own multi-bounce feedback
	// then carries it to OTHER probes that read this one -- an epidemic
	// spread, not a local glitch. Replacing with 0 instead of blending in
	// heals on the NEXT good frame rather than poisoning the history
	// permanently.
	if (any(isnan(irradiance)) || any(isinf(irradiance))) irradiance = 0;
	if (isnan(mean_dist)  || isinf(mean_dist))  mean_dist  = 0;
	if (isnan(mean_dist2) || isinf(mean_dist2)) mean_dist2 = 0;

	// Temporal blend toward last frame's convolved value (see
	// ddgi_probe_trace.hlsl's own comment on ray jitter): now that the
	// traced directions feeding this resample shift slightly every frame,
	// a single frame's result carries that jitter's own variance. Blending
	// it in gradually instead of overwriting outright turns that per-frame
	// variance into something the eye integrates over several frames rather
	// than a visible per-frame wobble -- exactly what the jitter is for
	// (spreading an occasional grazing-ray leak across time) actually
	// requires something downstream to average it, which nothing did
	// before this. probe_irradiance/probe_visibility are the same physical
	// resource DDGIProbeTrace reads as prev_irradiance/prev_visibility, so
	// reading them here before overwriting is exactly last frame's value --
	// no separate history buffer needed. A probe's first frame back from
	// being non-resident blends toward whatever its texels last held (its
	// own last valid value, or zero if truly never traced before), so it
	// ramps in over a handful of frames rather than snapping instantly --
	// an acceptable, self-correcting cost given how small the jitter is.
	const float blend_alpha = 0.15;
	float3 prev_irradiance  = data.GetProbe_irradiance()[uint3(local_texel, slice)].rgb;
	float2 prev_visibility  = data.GetProbe_visibility()[uint3(local_texel, slice)];

	// Same guard on the READ side -- self-heals an already-poisoned texel
	// (e.g. from before this fix landed, or from any other future source of
	// a bad value this doesn't otherwise catch) by refusing to blend TOWARD
	// a bad previous value at all; the fresh, already-verified-finite value
	// replaces it outright instead of lerping into a NaN sink forever.
	if (any(isnan(prev_irradiance)) || any(isinf(prev_irradiance))) prev_irradiance = irradiance;
	if (any(isnan(prev_visibility)) || any(isinf(prev_visibility))) prev_visibility = float2(mean_dist, mean_dist2);

	float3 blended_irradiance = lerp(prev_irradiance, irradiance, blend_alpha);
	float2 blended_visibility = lerp(prev_visibility, float2(mean_dist, mean_dist2), blend_alpha);

	data.GetProbe_irradiance()[uint3(local_texel, slice)] = float4(blended_irradiance, 1);
	data.GetProbe_visibility()[uint3(local_texel, slice)] = blended_visibility;
}
