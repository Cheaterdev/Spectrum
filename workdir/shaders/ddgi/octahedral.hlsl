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
