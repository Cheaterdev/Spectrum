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
// moved smoothly across it.
float4 ddgi_bilinear_texel4(Texture2D<float4> tex, uint2 origin, uint texel_size, float2 uv01)
{
	float2 texel_f = uv01 * texel_size - 0.5;
	float2 base = floor(texel_f);
	float2 frac = texel_f - base;
	int max_index = (int)texel_size - 1;

	uint2 t00 = origin + (uint2)clamp(int2(base) + int2(0, 0), 0, max_index);
	uint2 t10 = origin + (uint2)clamp(int2(base) + int2(1, 0), 0, max_index);
	uint2 t01 = origin + (uint2)clamp(int2(base) + int2(0, 1), 0, max_index);
	uint2 t11 = origin + (uint2)clamp(int2(base) + int2(1, 1), 0, max_index);

	float4 top = lerp(tex[t00], tex[t10], frac.x);
	float4 bot = lerp(tex[t01], tex[t11], frac.x);
	return lerp(top, bot, frac.y);
}

float2 ddgi_bilinear_texel2(Texture2D<float2> tex, uint2 origin, uint texel_size, float2 uv01)
{
	float2 texel_f = uv01 * texel_size - 0.5;
	float2 base = floor(texel_f);
	float2 frac = texel_f - base;
	int max_index = (int)texel_size - 1;

	uint2 t00 = origin + (uint2)clamp(int2(base) + int2(0, 0), 0, max_index);
	uint2 t10 = origin + (uint2)clamp(int2(base) + int2(1, 0), 0, max_index);
	uint2 t01 = origin + (uint2)clamp(int2(base) + int2(0, 1), 0, max_index);
	uint2 t11 = origin + (uint2)clamp(int2(base) + int2(1, 1), 0, max_index);

	float2 top = lerp(tex[t00], tex[t10], frac.x);
	float2 bot = lerp(tex[t01], tex[t11], frac.x);
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
	Texture2D<float4> probe_irradiance,
	Texture2D<float2> probe_visibility)
{
	uint texel_size = info.GetAtlas_info().x;
	uint3 probe_counts = info.GetProbe_counts().xyz;
	float3 grid_min = info.GetGrid_min().xyz;
	float3 spacing  = info.GetProbe_spacing().xyz;

	// No bound buffer -- only used for its pure coordinate-math helpers, same
	// reasoning as DDGIProbeConvolveData's own comment (ddgi.sig).
	DDGIProbes probes;

	float3 probe_space = (world_pos - grid_min) / spacing;
	float3 base = floor(probe_space);
	float3 frac_part = probe_space - base;

	float2 sample_uv = ddgi_oct_encode(normalize(normal)) * 0.5 + 0.5;

	float3 result = float3(0, 0, 0);
	float weight_sum = 0;

	for (uint i = 0; i < 8; i++)
	{
		float3 offset = float3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
		float3 coord = base + offset;

		if (any(coord < 0) || any(coord >= float3(probe_counts)))
			continue;

		float3 trilinear = lerp(1.0 - frac_part, frac_part, offset);
		float weight = trilinear.x * trilinear.y * trilinear.z;
		if (weight <= 0.0001)
			continue;

		uint3 probe_coord = uint3(coord);
		float3 probe_pos = probes.ddgi_probe_world_pos(probe_coord, grid_min, spacing, float3(0, 0, 0));
		// ddgi_atlas_origin is cascade-LOCAL (0..DDGI_AtlasWidth-1); add this
		// cascade's own X-offset (DDGIInfo::cascade_info.y) to land in its
		// slice of the shared, DDGI_CascadeCount-times-wider atlas.
		uint2 origin = probes.ddgi_atlas_origin(probe_coord, probe_counts.x, texel_size);
		origin.x += info.GetCascade_info().y;

		float3 to_point = world_pos - probe_pos;
		float dist_to_point = length(to_point);
		float3 dir_to_point = to_point / max(dist_to_point, 0.0001);

		// Backface rejection: down-weight a probe sitting on the far side of
		// the surface from its own normal -- e.g. a probe in the next room
		// through a thin wall is geometrically one of the 8 trilinear
		// corners even though it can never legitimately light this point.
		// Floored, not zeroed, matching the visibility floor below (a hard
		// cutoff produces a visible seam where a probe's weight snaps to 0).
		float backface = saturate(dot(normal, -dir_to_point) * 0.5 + 0.5);
		weight *= max(backface, 0.05);

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
		float2 vis = ddgi_bilinear_texel2(probe_visibility, origin, texel_size, vis_uv);

		float mean     = vis.x;
		float mean2    = vis.y;
		float variance = max(mean2 - mean * mean, 0.0001);
		float chebyshev = 1.0;
		if (dist_to_point > mean)
		{
			float diff = dist_to_point - mean;
			chebyshev = max(variance / (variance + diff * diff), 0.05);
		}
		weight *= chebyshev;

		result += ddgi_bilinear_texel4(probe_irradiance, origin, texel_size, sample_uv).rgb * weight;
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
	Texture2D<float4> probe_irradiance,
	Texture2D<float2> probe_visibility)
{
	DDGIInfo cascades[5] = { c0, c1, c2, c3, c4 };

	for (uint i = 0; i < 5; i++)
	{
		DDGIInfo info = cascades[i];
		float3 probe_space = (world_pos - info.GetGrid_min().xyz) / info.GetProbe_spacing().xyz;
		float3 counts = float3(info.GetProbe_counts().xyz);

		if (all(probe_space >= 1.0) && all(probe_space <= counts - 2.0))
			return ddgi_sample_irradiance(world_pos, normal, info, probe_irradiance, probe_visibility);
	}

	return ddgi_sample_irradiance(world_pos, normal, c4, probe_irradiance, probe_visibility);
}
