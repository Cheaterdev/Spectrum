#pragma once

// Signed-octahedron unit-direction encoding (Cigolle et al., "A Survey of
// Efficient Representations for Independent Unit Vectors"), used to lay
// per-probe radiance/irradiance/visibility out as flat 2D atlas texels (see
// [[project-ddgi]] planning notes). uv is in [-1,1], not [0,1] -- callers
// convert to/from texel space themselves (DDGIProbes::ddgi_atlas_origin /
// ddgi_atlas_local_uv, ddgi.sig).

float3 ddgi_oct_decode(float2 uv)
{
	float3 n = float3(uv.x, uv.y, 1.0 - abs(uv.x) - abs(uv.y));
	float t = saturate(-n.z);
	// DXC rejects a vector-conditioned `?:` ("must be scalar, use 'select'")
	// -- select() is the vector-condition equivalent.
	n.xy += select(n.xy >= 0.0, -t.xx, t.xx);
	return normalize(n);
}

float2 ddgi_oct_encode(float3 n)
{
	float2 p = n.xy * (1.0 / (abs(n.x) + abs(n.y) + abs(n.z)));
	if (n.z <= 0.0)
	{
		float2 signs = float2(p.x >= 0.0 ? 1.0 : -1.0, p.y >= 0.0 ? 1.0 : -1.0);
		p = (1.0 - abs(p.yx)) * signs;
	}
	return p;
}

// Spherical Fibonacci point set (Keinert et al., "Spherical Fibonacci
// Mapping") -- a deterministic, near-uniform distribution of `count`
// directions over the sphere, computed purely from `index`/`count`: no
// precomputed direction table, no per-probe storage. DDGIProbeTrace fires
// exactly one ray per index (ddgi.sig's DDGI_ProbeRayCount comment on why
// this replaced 1-ray-per-octahedral-texel), and DDGIProbeConvolve
// recomputes the SAME direction from the same index when resampling that
// ray's stored radiance into the octahedral output map -- the two must stay
// in lockstep, which a shared pure function guarantees for free (no risk of
// trace and convolve drifting onto different direction sets).
float3 ddgi_sphere_fibonacci(uint index, uint count)
{
	// Local, not common.hlsl's PI -- this header is included by files that
	// don't otherwise need common.hlsl, and in an inclusion order relative
	// to it that isn't guaranteed (HLSL has no include guards against
	// use-before-declare the way a forward-declared symbol would need,
	// confirmed the hard way: "use of undeclared identifier 'PI'" when
	// common.hlsl happened to be included AFTER this file). Self-contained
	// instead of depending on caller include order.
	const float two_pi = 6.283185307179586;
	const float golden_ratio = 1.618033988749895; // (1 + sqrt(5)) / 2
	float fi = float(index);
	float fn = max(float(count), 1.0);
	float phi = two_pi * frac(fi * (golden_ratio - 1.0));
	float cos_theta = 1.0 - (2.0 * fi + 1.0) / fn;
	float sin_theta = sqrt(saturate(1.0 - cos_theta * cos_theta));
	return float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}
