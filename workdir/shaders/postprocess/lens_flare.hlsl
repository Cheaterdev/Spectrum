// Image-based lens flare. Ghosts and halo follow John Chapman's "Pseudo Lens
// Flare"; streaks are Masaki Kawase's streak filter (GDC 2003). The source is
// Bloom's thresholded downsample chain, so only what blooms flares.

#include "../autogen/layout/DefaultLayout.h"

// Fades samples toward the source image border so ghosts of off-screen or
// edge content don't cut off hard.
float flare_window(float2 q)
{
	return pow(saturate(1.0 - length(q - 0.5) / 0.7071), 5);
}

#ifdef BUILD_FUNC_CS_Ghosts
#include "../autogen/LensFlareGhosts.h"

// A ghost is the source image scaled by 1/k about the screen center (negative k
// mirrors it through the center). |k| > 1 ghosts are magnified and so spread
// their light thinner -- the smaller weights keep them from dominating.
static const int GhostCount = 8;
static const float GhostScale[GhostCount]  = { -0.3, -0.5, -0.75, -1.1, -1.6, -2.4, 0.45, 1.8 };
static const float GhostWeight[GhostCount] = { 1.0, 0.8, 0.6, 0.5, 0.4, 0.3, 0.5, 0.25 };
// Anti-reflection coatings give ghosts their color casts.
static const float3 GhostTint[GhostCount] =
{
	float3(1.0, 0.6, 0.25),
	float3(0.4, 1.0, 0.5),
	float3(0.7, 0.4, 1.0),
	float3(0.3, 0.8, 1.0),
	float3(1.0, 0.5, 0.2),
	float3(0.4, 0.9, 0.8),
	float3(1.0, 0.4, 0.8),
	float3(0.4, 0.6, 1.0),
};

float3 sample_scaled(Texture2D<float4> src, float2 uv, float k, float chromatic)
{
	float3 ks = k * (1.0 + chromatic * float3(-1, 0, 1));
	float3 c;
	[unroll]
	for (int ch = 0; ch < 3; ch++)
	{
		float2 q = 0.5 + (uv - 0.5) / ks[ch];
		c[ch] = src.SampleLevel(linearClampSampler, q, 0)[ch] * flare_window(q);
	}
	return c;
}

[numthreads(8, 8, 1)]
void CS_Ghosts(uint3 id : SV_DispatchThreadID)
{
	const LensFlareGhosts data = GetLensFlareGhosts();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	Texture2D<float4> src = data.GetSource();
	float2 uv = (id.xy + 0.5) / float2(size);
	float chromatic = data.GetChromatic();

	float3 ghosts = 0;
	[unroll]
	for (int i = 0; i < GhostCount; i++)
		ghosts += sample_scaled(src, uv, GhostScale[i], chromatic) * GhostTint[i] * GhostWeight[i];

	// Halo: each pixel samples a fixed distance toward the screen center, so a
	// source anywhere lights an arc of the ring of that radius around the
	// center, on its side. Built in aspect-corrected space to stay circular.
	float2 aspect = float2(data.GetAspect(), 1);
	float2 v = (uv - 0.5) * aspect;
	float2 dir = length(v) > 1e-5 ? -normalize(v) : float2(0, 0);
	float3 radius = data.GetHalo_radius() * (1.0 + chromatic * float3(-1, 0, 1));
	float3 halo;
	[unroll]
	for (int ch = 0; ch < 3; ch++)
	{
		float2 q = uv + dir * radius[ch] / aspect;
		halo[ch] = src.SampleLevel(linearClampSampler, q, 0)[ch] * flare_window(q);
	}

	target[id.xy] = float4(ghosts * data.GetGhost_intensity() + halo * data.GetHalo_intensity(), 1);
}
#endif

#ifdef BUILD_FUNC_CS_Streak
#include "../autogen/LensFlareStreak.h"

[numthreads(8, 8, 1)]
void CS_Streak(uint3 id : SV_DispatchThreadID)
{
	const LensFlareStreak data = GetLensFlareStreak();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	Texture2D<float4> src = data.GetSource();
	float2 texel = data.GetTexel();
	float2 uv = (id.xy + 0.5) * texel;
	float2 step_uv = data.GetDirection() * texel * data.GetStep();

	// Tapered, overlapping taps instead of a flat 4-tap box: a box makes every
	// level a plateau that ends abruptly, and summing plateaus of different
	// lengths shows as visible steps. Half spacing overlaps the previous
	// level's extent, and the geometric weights fade each level to its end.
	float3 c = 0;
	float wsum = 0;
	[unroll]
	for (int s = 0; s < 8; s++)
	{
		float w = pow(0.8, s);
		c += w * src.SampleLevel(linearClampSampler, uv + step_uv * (s * 0.5), 0).rgb;
		wsum += w;
	}
	c /= wsum;

	target[id.xy] = float4(c, 1);

	RWTexture2D<float4> accum = data.GetAccum();
	float3 a = c * data.GetAccum_weight();
	if (data.GetAccumulate() != 0)
		a += accum[id.xy].rgb;
	accum[id.xy] = float4(a, 1);
}
#endif
