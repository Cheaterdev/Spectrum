#pragma once

#include "../autogen/tables/DDGIInfo.h"
#include "../autogen/tables/DDGIProbes.h"
#include "octahedral.hlsl"

// Manual bilinear read within a single probe's own texel_size x texel_size
// octahedral cell, clamped to that cell's own bounds (not wrapped across
// the octahedral fold seam -- a real border-padded wrap, like AC Shadows'
// own probe atlas uses, is a further follow-up; clamping just means a
// small handful of directions right at the seam interpolate slightly wrong
// instead of bleeding into the next probe's unrelated data). Needed because
// each probe's map is only texel_size^2 texels -- nearest-sampling it made
// the result visibly blocky ("mosaic") as the shading direction/position
// moved smoothly across it. `slice` selects the array layer (probe_y +
// cascade offset, see ddgi_atlas_array_slice, ddgi.sig) -- the 2D atlas
// plane only ever holds (probe_x, probe_z).
float4 ddgi_bilinear_texel4(Texture2DArray<float4> tex, uint2 origin, uint slice, uint texel_size, float2 uv01)
{
	float2 texel_f = uv01 * texel_size - 0.5;
	float2 base = floor(texel_f);
	float2 frac = texel_f - base;
	int max_index = (int)texel_size - 1;

	uint2 t00 = origin + (uint2)clamp(int2(base) + int2(0, 0), 0, max_index);
	uint2 t10 = origin + (uint2)clamp(int2(base) + int2(1, 0), 0, max_index);
	uint2 t01 = origin + (uint2)clamp(int2(base) + int2(0, 1), 0, max_index);
	uint2 t11 = origin + (uint2)clamp(int2(base) + int2(1, 1), 0, max_index);

	float4 top = lerp(tex[uint3(t00, slice)], tex[uint3(t10, slice)], frac.x);
	float4 bot = lerp(tex[uint3(t01, slice)], tex[uint3(t11, slice)], frac.x);
	return lerp(top, bot, frac.y);
}

float2 ddgi_bilinear_texel2(Texture2DArray<float2> tex, uint2 origin, uint slice, uint texel_size, float2 uv01)
{
	float2 texel_f = uv01 * texel_size - 0.5;
	float2 base = floor(texel_f);
	float2 frac = texel_f - base;
	int max_index = (int)texel_size - 1;

	uint2 t00 = origin + (uint2)clamp(int2(base) + int2(0, 0), 0, max_index);
	uint2 t10 = origin + (uint2)clamp(int2(base) + int2(1, 0), 0, max_index);
	uint2 t01 = origin + (uint2)clamp(int2(base) + int2(0, 1), 0, max_index);
	uint2 t11 = origin + (uint2)clamp(int2(base) + int2(1, 1), 0, max_index);

	float2 top = lerp(tex[uint3(t00, slice)], tex[uint3(t10, slice)], frac.x);
	float2 bot = lerp(tex[uint3(t01, slice)], tex[uint3(t11, slice)], frac.x);
	return lerp(top, bot, frac.y);
}

// v1 (see [[project-ddgi]] planning notes): samples the probe-volume
// irradiance field at a world position + surface normal, trilinearly
// blending the 8 surrounding probes with a simplified chebyshev visibility
// weight AND a backface weight (Majercik et al., "Dynamic Diffuse Global
// Illumination") to reduce light leaking through walls/thin occluders.
float3 ddgi_sample_irradiance(
	float3 world_pos, float3 normal,
	DDGIInfo info,
	Texture2DArray<float4> probe_irradiance,
	Texture2DArray<float2> probe_visibility,
	StructuredBuffer<uint> probe_residency)
{
	uint texel_size = info.GetAtlas_info().x;
	uint3 probe_counts = info.GetProbe_counts().xyz;
	float3 spacing  = info.GetProbe_spacing().xyz;

	// No bound buffer -- only used for its pure coordinate-math helpers, same
	// reasoning as DDGIProbeConvolveData's own comment (ddgi.sig).
	DDGIProbes probes;

	// Absolute (grid_min-independent) cell + fractional part -- see
	// ddgi_world_to_slot's own comment (ddgi.sig) for why this must be a
	// pure function of world_pos/spacing alone, not (world_pos-grid_min)/
	// spacing: subtracting grid_min first doesn't change the fractional
	// part (floor(x-n) = floor(x)-n for integer n, so frac is identical
	// either way), but it WOULD change which absolute cell a wrapped slot
	// reconstructs to depending on the window's current position -- exactly
	// the bug toroidal addressing exists to avoid.
	float3 probe_space = world_pos / spacing;
	float3 base = floor(probe_space);
	float3 frac_part = probe_space - base;

	float2 sample_uv = ddgi_oct_encode(normalize(normal)) * 0.5 + 0.5;

	float3 result = float3(0, 0, 0);
	float weight_sum = 0;

	for (uint i = 0; i < 8; i++)
	{
		float3 offset = float3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
		float3 absolute_corner = base + offset;

		float3 trilinear = lerp(1.0 - frac_part, frac_part, offset);
		float weight = trilinear.x * trilinear.y * trilinear.z;
		if (weight <= 0.0001)
			continue;

		// Toroidal wrap of the ABSOLUTE cell (see ddgi_wrap/ddgi_world_to_slot's
		// own comments, ddgi.sig) -- always valid, no in-grid check needed:
		// ddgi_sample_irradiance_cascaded's own margin check already
		// guarantees world_pos (and therefore every one of its 8 corners)
		// sits well inside this cascade's current window before this
		// function is ever called.
		uint3 probe_coord = uint3(probes.ddgi_wrap(int3(absolute_corner), probe_counts));

		// Skip a probe DDGIProbeResidencyMark hasn't marked needed -- its
		// atlas texels are stale (last valid value before it was culled) or
		// were never written at all if it's never been resident, either way
		// not something to blend in as if it were current. Not counting its
		// weight toward weight_sum means the remaining resident corners'
		// weights renormalize on their own (see the final result/weight_sum
		// below).
		uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, probe_counts);
		if (probe_residency[info.GetCascade_info().x + probe_linear_index] == 0)
			continue;

		// Exact -- already have the real absolute cell in hand, no need to
		// reconstruct it back from the wrapped slot (ddgi_probe_world_pos's
		// own job, for callers that start from a bare slot instead).
		float3 probe_pos = absolute_corner * spacing;
		// ddgi_atlas_origin gives the (x,z) plane position; probe_coord.y
		// plus this cascade's own slice offset (DDGIInfo::cascade_info.y)
		// gives which array slice of the shared atlas holds it -- see
		// ddgi_atlas_array_slice's own comment (ddgi.sig).
		uint2 origin = probes.ddgi_atlas_origin(probe_coord, texel_size);
		uint slice = probes.ddgi_atlas_array_slice(probe_coord.y, info.GetCascade_info().y);

		float3 to_point = world_pos - probe_pos;
		float dist_to_point = length(to_point);
		float3 dir_to_point = to_point / max(dist_to_point, 0.0001);

		// Backface rejection: down-weight a probe sitting on the far side of
		// the surface from its own normal -- e.g. a probe in the next room
		// through a thin wall is geometrically one of the 8 trilinear
		// corners even though it can never legitimately light this point.
		// Floored, not zeroed, matching the visibility floor below (a hard
		// cutoff produces a visible seam where a probe's weight snaps to 0).
		float backface = saturate(dot(normal, -dir_to_point) <0);
	//weight *= max(backface, 0.05);

		// Visibility must be sampled in the direction FROM the probe TO the
		// shading point -- that's what DDGIProbeTrace's own visibility
		// texel actually measures (mean/mean-square hit distance of rays
		// the probe traced outward in ddgi_oct_decode(local_uv) directions,
		// same convention). Sampling it at the surface normal's direction
		// instead (an earlier bug here) tests "what did the probe see
		// looking the same way this surface faces", not "is there a wall
		// between the probe and this specific point" -- the actual
		// occlusion test barely rejected anything, which is what let light
		// leak through walls.
		float2 vis_uv = ddgi_oct_encode(dir_to_point) * 0.5 + 0.5;
		float2 vis = ddgi_bilinear_texel2(probe_visibility, origin, slice, texel_size, vis_uv);

		float mean     = vis.x;
		float mean2    = vis.y;
		float variance = max(mean2 - mean * mean, 0.0001);
		float chebyshev = 1.0;
		if (dist_to_point > mean)
		{
			float diff = dist_to_point - mean;
			chebyshev = max(variance / (variance + diff * diff), 0.05);
		}
		//weight *= chebyshev;

		result += ddgi_bilinear_texel4(probe_irradiance, origin, slice, texel_size, sample_uv).rgb * weight;
		weight_sum += weight;
	}

	return weight_sum > 0.0001 ? result / weight_sum : float3(0, 0, 0);
}

// Tries each cascade from finest (c0) to coarsest (c4), using the first
// whose grid actually contains world_pos with a full 1-probe margin on
// every side (so the 8-probe trilinear sample never needs to reach outside
// that cascade's own grid). Falls back to the coarsest cascade even if the
// point technically falls just outside its margin -- an imperfect sample at
// the very edge beats a hard black cutoff. No cross-cascade blending yet
// (deferred): a shading point right at a cascade boundary can show a subtle
// seam where the picked cascade switches, same class of simplification as
// DDGIProbeConvolve's own naive (non-LDS-prefiltered) convolution.
float3 ddgi_sample_irradiance_cascaded(
	float3 world_pos, float3 normal,
	DDGIInfo c0, DDGIInfo c1, DDGIInfo c2, DDGIInfo c3, DDGIInfo c4,
	Texture2DArray<float4> probe_irradiance,
	Texture2DArray<float2> probe_visibility,
	StructuredBuffer<uint> probe_residency)
{
	DDGIInfo cascades[5] = { c0, c1, c2, c3, c4 };

	for (uint i = 0; i < 5; i++)
	{
		DDGIInfo info = cascades[i];
		float3 probe_space = (world_pos - info.GetGrid_min().xyz) / info.GetProbe_spacing().xyz;
		float3 counts = float3(info.GetProbe_counts().xyz);

		if (all(probe_space >= 1.0) && all(probe_space <= counts - 2.0))
			return ddgi_sample_irradiance(world_pos, normal, info, probe_irradiance, probe_visibility, probe_residency);
	}

	return ddgi_sample_irradiance(world_pos, normal, c4, probe_irradiance, probe_visibility, probe_residency);
}

// Real occlusion test between a shading point and a probe, in place of the
// chebyshev/variance heuristic above -- that heuristic bilinearly
// interpolates raw (mean, mean^2) moments across 4 texels before computing
// variance from the interpolated pair (ddgi_bilinear_texel2), which is a
// well-known "light bleeding" failure mode right at any boundary where two
// texels see meaningfully different hit distances -- exactly where
// occlusion correctness matters most. This fires an actual ray instead:
// correct, but real cost -- one extra TraceRay per trilinear corner per
// sample, unlike the heuristic's plain texture read. Only usable from a
// raygen shader (TraceRay isn't valid syntax anywhere else) -- a caller
// must `#define DDGI_SAMPLE_ENABLE_TRACED_VISIBILITY` before including this
// file to get it; ddgi_indirect_debug.hlsl (a compute shader, can't call
// TraceRay at all) doesn't define it and only ever sees the untraced
// functions above.
#ifdef DDGI_SAMPLE_ENABLE_TRACED_VISIBILITY
#include "../autogen/tables/ShadowPayload.h"
#include "../autogen/rtx/ShadowPass.h"

float ddgi_probe_visibility_ray(RaytracingAccelerationStructure scene, float3 world_pos, float3 probe_pos)
{
	float3 to_probe = probe_pos - world_pos;
	float dist = length(to_probe);
	if (dist < 0.001)
		return 1.0;
	float3 dir = to_probe / dist;

	// Small bias off the surface on both ends so the ray doesn't
	// self-intersect the shading point's own geometry or overshoot past
	// the probe.
	ShadowPayload payload_shadow = { false, 0 };

	RayDesc ray;
	ray.Origin = world_pos + dir * 0.02;
	ray.Direction = dir;
	ray.TMin = 0.0;
	ray.TMax = max(dist - 0.04, 0.0);
	ShadowPass(scene, ray, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, payload_shadow);

	return payload_shadow.hit ? 0.0 : 1.0;
}

// Same 8-probe trilinear blend as ddgi_sample_irradiance above, but weights
// each corner by a real traced visibility ray instead of the chebyshev
// heuristic -- probe_visibility is kept in the signature purely so callers
// can switch between this and the untraced version without reshaping their
// own call site, even though this version never reads it.
float3 ddgi_sample_irradiance_traced(
	float3 world_pos, float3 normal,
	DDGIInfo info,
	Texture2DArray<float4> probe_irradiance,
	Texture2DArray<float2> probe_visibility,
	StructuredBuffer<uint> probe_residency,
	RaytracingAccelerationStructure scene)
{
	uint texel_size = info.GetAtlas_info().x;
	uint3 probe_counts = info.GetProbe_counts().xyz;
	float3 spacing  = info.GetProbe_spacing().xyz;

	DDGIProbes probes;

	// Absolute cell + fractional part -- see ddgi_sample_irradiance's own
	// comment on the identical block (this file) for why this must NOT
	// subtract grid_min first.
	float3 probe_space = world_pos / spacing;
	float3 base = floor(probe_space);
	float3 frac_part = probe_space - base;

	float2 sample_uv = ddgi_oct_encode(normalize(normal)) * 0.5 + 0.5;

	float3 result = float3(0, 0, 0);
	float weight_sum = 0;

	for (uint i = 0; i < 8; i++)
	{
		float3 offset = float3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
		float3 absolute_corner = base + offset;

		float3 trilinear = lerp(1.0 - frac_part, frac_part, offset);
		float weight = trilinear.x * trilinear.y * trilinear.z;
		if (weight <= 0.0001)
			continue;

		// Toroidal wrap of the absolute cell -- see ddgi_sample_irradiance's
		// own comment on the identical block (this file).
		uint3 probe_coord = uint3(probes.ddgi_wrap(int3(absolute_corner), probe_counts));

		uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, probe_counts);
		if (probe_residency[info.GetCascade_info().x + probe_linear_index] == 0)
			continue;

		float3 probe_pos = absolute_corner * spacing;

		float3 to_point = world_pos - probe_pos;
		float dist_to_point = length(to_point);
		float3 dir_to_point = to_point / max(dist_to_point, 0.0001);

		// Backface rejection stays as a cheap pre-filter -- skip firing a
		// ray at all for a probe already known to be on the wrong side.
		float backface = saturate(dot(normal, -dir_to_point) * 0.5 + 0.5);
		weight *= max(backface, 0.05);
		if (weight <= 0.0001)
			continue;

		weight *= ddgi_probe_visibility_ray(scene, world_pos, probe_pos);
		if (weight <= 0.0001)
			continue;

		uint2 origin = probes.ddgi_atlas_origin(probe_coord, texel_size);
		uint slice = probes.ddgi_atlas_array_slice(probe_coord.y, info.GetCascade_info().y);

		result += ddgi_bilinear_texel4(probe_irradiance, origin, slice, texel_size, sample_uv).rgb * weight;
		weight_sum += weight;
	}

	return weight_sum > 0.0001 ? result / weight_sum : float3(0, 0, 0);
}

float3 ddgi_sample_irradiance_cascaded_traced(
	float3 world_pos, float3 normal,
	DDGIInfo c0, DDGIInfo c1, DDGIInfo c2, DDGIInfo c3, DDGIInfo c4,
	Texture2DArray<float4> probe_irradiance,
	Texture2DArray<float2> probe_visibility,
	StructuredBuffer<uint> probe_residency,
	RaytracingAccelerationStructure scene)
{
	DDGIInfo cascades[5] = { c0, c1, c2, c3, c4 };

	for (uint i = 0; i < 5; i++)
	{
		DDGIInfo info = cascades[i];
		float3 probe_space = (world_pos - info.GetGrid_min().xyz) / info.GetProbe_spacing().xyz;
		float3 counts = float3(info.GetProbe_counts().xyz);

		if (all(probe_space >= 1.0) && all(probe_space <= counts - 2.0))
			return ddgi_sample_irradiance_traced(world_pos, normal, info, probe_irradiance, probe_visibility, probe_residency, scene);
	}

	return ddgi_sample_irradiance_traced(world_pos, normal, c4, probe_irradiance, probe_visibility, probe_residency, scene);
}
#endif // DDGI_SAMPLE_ENABLE_TRACED_VISIBILITY

// Same occlusion test as ddgi_probe_visibility_ray above, but via DXR Tier
// 1.1 inline ray tracing (RayQuery) instead of classic TraceRay -- usable
// from ANY shader stage, including a plain compute shader like
// ddgi_indirect_debug.hlsl, which can't call TraceRay/use ShadowPass at all
// (no raygen/hit/miss pipeline to run it in). Kept separate from
// ddgi_probe_visibility_ray/ddgi_sample_irradiance_traced above rather than
// replacing them, so the already-validated raygen call sites (DDGIProbeTrace,
// TraceIndirectDiffuse) aren't touched by this. No any-hit/alpha-test
// handling -- COMMITTED_TRIANGLE_HIT treats every triangle as instantly
// opaque, same simplifying assumption RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
// already implies for a fast approximate occlusion test.
#ifdef DDGI_SAMPLE_ENABLE_INLINE_TRACED_VISIBILITY
float ddgi_probe_visibility_ray_inline(RaytracingAccelerationStructure scene, float3 world_pos, float3 probe_pos)
{
	float3 to_probe = probe_pos - world_pos;
	float dist = length(to_probe);
	if (dist < 0.001)
		return 1.0;
	float3 dir = to_probe / dist;

	RayDesc ray;
	ray.Origin = world_pos + dir * 0.02;
	ray.Direction = dir;
	ray.TMin = 0.0;
	ray.TMax = max(dist - 0.04, 0.0);

	RayQuery<RAY_FLAG_NONE> q;
	q.TraceRayInline(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, ray);
	q.Proceed();

	return (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) ? 0.0 : 1.0;
}

// Same shape as ddgi_sample_irradiance_traced above, just calling the
// inline-query occlusion test instead of the classic-TraceRay one.
float3 ddgi_sample_irradiance_inline_traced(
	float3 world_pos, float3 normal,
	DDGIInfo info,
	Texture2DArray<float4> probe_irradiance,
	Texture2DArray<float2> probe_visibility,
	StructuredBuffer<uint> probe_residency,
	RaytracingAccelerationStructure scene)
{
	uint texel_size = info.GetAtlas_info().x;
	uint3 probe_counts = info.GetProbe_counts().xyz;
	float3 spacing  = info.GetProbe_spacing().xyz;

	DDGIProbes probes;

	// Absolute cell + fractional part -- see ddgi_sample_irradiance's own
	// comment on the identical block (this file) for why this must NOT
	// subtract grid_min first.
	float3 probe_space = world_pos / spacing;
	float3 base = floor(probe_space);
	float3 frac_part = probe_space - base;

	float2 sample_uv = ddgi_oct_encode(normalize(normal)) * 0.5 + 0.5;

	float3 result = float3(0, 0, 0);
	float weight_sum = 0;

	for (uint i = 0; i < 8; i++)
	{
		float3 offset = float3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
		float3 absolute_corner = base + offset;

		float3 trilinear = lerp(1.0 - frac_part, frac_part, offset);
		float weight = trilinear.x * trilinear.y * trilinear.z;
		if (weight <= 0.0001)
			continue;

		// Toroidal wrap of the absolute cell -- see ddgi_sample_irradiance's
		// own comment on the identical block (this file).
		uint3 probe_coord = uint3(probes.ddgi_wrap(int3(absolute_corner), probe_counts));

		uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, probe_counts);
		if (probe_residency[info.GetCascade_info().x + probe_linear_index] == 0)
			continue;

		float3 probe_pos = absolute_corner * spacing;

		float3 to_point = world_pos - probe_pos;
		float dist_to_point = length(to_point);
		float3 dir_to_point = to_point / max(dist_to_point, 0.0001);

		float backface = saturate(dot(normal, -dir_to_point) * 0.5 + 0.5);
		weight *= max(backface, 0.05);
		if (weight <= 0.0001)
			continue;

		weight *= ddgi_probe_visibility_ray_inline(scene, world_pos, probe_pos);
		if (weight <= 0.0001)
			continue;

		uint2 origin = probes.ddgi_atlas_origin(probe_coord, texel_size);
		uint slice = probes.ddgi_atlas_array_slice(probe_coord.y, info.GetCascade_info().y);

		result += ddgi_bilinear_texel4(probe_irradiance, origin, slice, texel_size, sample_uv).rgb * weight;
		weight_sum += weight;
	}

	return weight_sum > 0.0001 ? result / weight_sum : float3(0, 0, 0);
}

float3 ddgi_sample_irradiance_cascaded_inline_traced(
	float3 world_pos, float3 normal,
	DDGIInfo c0, DDGIInfo c1, DDGIInfo c2, DDGIInfo c3, DDGIInfo c4,
	Texture2DArray<float4> probe_irradiance,
	Texture2DArray<float2> probe_visibility,
	StructuredBuffer<uint> probe_residency,
	RaytracingAccelerationStructure scene)
{
	DDGIInfo cascades[5] = { c0, c1, c2, c3, c4 };

	for (uint i = 0; i < 5; i++)
	{
		DDGIInfo info = cascades[i];
		float3 probe_space = (world_pos - info.GetGrid_min().xyz) / info.GetProbe_spacing().xyz;
		float3 counts = float3(info.GetProbe_counts().xyz);

		if (all(probe_space >= 1.0) && all(probe_space <= counts - 2.0))
			return ddgi_sample_irradiance_inline_traced(world_pos, normal, info, probe_irradiance, probe_visibility, probe_residency, scene);
	}

	return ddgi_sample_irradiance_inline_traced(world_pos, normal, c4, probe_irradiance, probe_visibility, probe_residency, scene);
}
#endif // DDGI_SAMPLE_ENABLE_INLINE_TRACED_VISIBILITY
