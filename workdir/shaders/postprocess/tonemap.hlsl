#include "../autogen/TonemapData.h"

#define HISTOGRAM_BINS 256

float luminance(float3 c)
{
	return dot(c, float3(0.2126, 0.7152, 0.0722));
}

uint luminance_to_bin(float lum, float min_log_lum, float inv_log_lum_range)
{
	if (lum < 1e-5)
		return 0;
	float t = saturate((log2(lum) - min_log_lum) * inv_log_lum_range);
	return (uint)(t * 254.0 + 1.0);
}

float bin_to_log_luminance(uint bin, float min_log_lum, float log_lum_range)
{
	return (max(bin, 1u) - 1.0) / 254.0 * log_lum_range + min_log_lum;
}

[numthreads(HISTOGRAM_BINS, 1, 1)]
void CS_Clear(uint gi : SV_GroupIndex)
{
	GetTonemapData().GetHistogram()[gi] = 0;
}

groupshared uint g_bins[HISTOGRAM_BINS];

[numthreads(16, 16, 1)]
void CS_Histogram(uint3 id : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
	const TonemapData data = GetTonemapData();

	g_bins[gi] = 0;
	GroupMemoryBarrierWithGroupSync();

	RWTexture2D<float4> color = data.GetColor();
	uint2 size;
	color.GetDimensions(size.x, size.y);

	if (all(id.xy < size))
	{
		float lum = luminance(color[id.xy].rgb);
		InterlockedAdd(g_bins[luminance_to_bin(lum, data.GetMin_log_lum(), 1.0 / data.GetLog_lum_range())], 1);
	}
	GroupMemoryBarrierWithGroupSync();

	if (g_bins[gi] != 0)
		InterlockedAdd(data.GetHistogram()[gi], g_bins[gi]);
}

groupshared float g_prefix[HISTOGRAM_BINS];
groupshared float g_weighted[HISTOGRAM_BINS];
groupshared float g_counts[HISTOGRAM_BINS];

[numthreads(HISTOGRAM_BINS, 1, 1)]
void CS_Adapt(uint gi : SV_GroupIndex)
{
	const TonemapData data = GetTonemapData();

	float count = (float)data.GetHistogram()[gi];
	g_prefix[gi] = count;
	GroupMemoryBarrierWithGroupSync();

	// Inclusive Hillis-Steele scan: after this, g_prefix[i] = sum of bins 0..i.
	[unroll]
	for (uint offset = 1; offset < HISTOGRAM_BINS; offset <<= 1)
	{
		float v = gi >= offset ? g_prefix[gi - offset] : 0;
		GroupMemoryBarrierWithGroupSync();
		g_prefix[gi] += v;
		GroupMemoryBarrierWithGroupSync();
	}

	float total = g_prefix[HISTOGRAM_BINS - 1];
	float lo    = total * data.GetLow_percent();
	float hi    = total * data.GetHigh_percent();

	// Only the part of this bin's pixel range [prefix - count, prefix] that
	// falls inside the percentile window [lo, hi] meters.
	float bin_end   = g_prefix[gi];
	float bin_start = bin_end - count;
	float clipped   = max(0.0, min(bin_end, hi) - max(bin_start, lo));

	g_counts[gi]   = clipped;
	g_weighted[gi] = clipped * bin_to_log_luminance(gi, data.GetMin_log_lum(), data.GetLog_lum_range());
	GroupMemoryBarrierWithGroupSync();

	[unroll]
	for (uint stride = HISTOGRAM_BINS / 2; stride > 0; stride >>= 1)
	{
		if (gi < stride)
		{
			g_counts[gi]   += g_counts[gi + stride];
			g_weighted[gi] += g_weighted[gi + stride];
		}
		GroupMemoryBarrierWithGroupSync();
	}

	if (gi != 0)
		return;

	RWStructuredBuffer<float4> state = data.GetExposure_state();

	float adapted;
	if (data.GetAuto_exposure() == 0)
	{
		adapted = -data.GetManual_ev();
	}
	else
	{
		float target = g_counts[0] > 0 ? g_weighted[0] / g_counts[0] : 0;
		target = clamp(target, data.GetMin_ev(), data.GetMax_ev());

		float prev = state[0].x;
		if (data.GetReset() != 0 || !isfinite(prev))
		{
			adapted = target;
		}
		else
		{
			float speed = target > prev ? data.GetSpeed_up() : data.GetSpeed_down();
			adapted = prev + (target - prev) * (1.0 - exp(-data.GetDelta_time() * speed));
		}
	}

	// 0.18 = middle grey: the metered average luminance maps to 18% reflectance.
	float exposure = 0.18 * exp2(data.GetExposure_compensation() - adapted);
	state[0] = float4(adapted, exposure, 0, 0);
}

// AgX, minimal approximation by Benjamin Wrensch
// (https://iolite-engine.com/blog_posts/minimal_agx_implementation).
float3 agx_contrast_approx(float3 x)
{
	float3 x2 = x * x;
	float3 x4 = x2 * x2;
	return 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
}

float3 tonemap_agx(float3 c)
{
	// Matrices are transcribed verbatim from the GLSL original, whose mat3
	// constructor is column-major; mul(v, M) with HLSL's row-major float3x3
	// reproduces GLSL's M * v. Swapping to mul(M, v) silently transposes them.
	const float3x3 agx_mat = float3x3(
		0.842479062253094, 0.0423282422610123, 0.0423756549057051,
		0.0784335999999992, 0.878468636469772, 0.0784336,
		0.0792237451477643, 0.0791661274605434, 0.879142973793104);
	const float3x3 agx_mat_inv = float3x3(
		1.19687900512017, -0.0528968517574562, -0.0529716355144438,
		-0.0980208811401368, 1.15190312990417, -0.0980434501171241,
		-0.0990297440797205, -0.0989611768448433, 1.15107367264116);
	const float min_ev = -12.47393;
	const float max_ev = 4.026069;

	c = mul(max(c, 1e-10), agx_mat);
	c = clamp(log2(c), min_ev, max_ev);
	c = (c - min_ev) / (max_ev - min_ev);
	c = agx_contrast_approx(c);
	c = mul(c, agx_mat_inv);
	// AgX's output is display-encoded (2.2); return it linear because the
	// viewport widget re-encodes.
	return pow(saturate(c), 2.2);
}

// Khronos PBR Neutral (https://github.com/KhronosGroup/ToneMapping).
float3 tonemap_pbr_neutral(float3 c)
{
	const float start_compression = 0.8 - 0.04;
	const float desaturation = 0.15;

	float x = min(c.r, min(c.g, c.b));
	float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
	c -= offset;

	float peak = max(c.r, max(c.g, c.b));
	if (peak < start_compression)
		return c;

	const float d = 1.0 - start_compression;
	float new_peak = 1.0 - d * d / (peak + d - start_compression);
	c *= new_peak / peak;

	float g = 1.0 - 1.0 / (desaturation * (peak - new_peak) + 1.0);
	return lerp(c, new_peak.xxx, g);
}

[numthreads(8, 8, 1)]
void CS_Apply(uint3 id : SV_DispatchThreadID)
{
	const TonemapData data = GetTonemapData();

	RWTexture2D<float4> color = data.GetColor();
	uint2 size;
	color.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	float4 c = color[id.xy];
	float3 rgb = max(c.rgb, 0) * data.GetExposure_state()[0].y;

	if (data.GetTonemap_operator() == TonemapOperator::AgX)
		rgb = tonemap_agx(rgb);
	else if (data.GetTonemap_operator() == TonemapOperator::PBRNeutral)
		rgb = tonemap_pbr_neutral(rgb);

	color[id.xy] = float4(saturate(rgb), c.a);
}
