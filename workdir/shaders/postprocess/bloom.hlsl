// Jimenez 2014, "Next Generation Post Processing in Call of Duty: Advanced
// Warfare" -- 13-tap downsample (with Karis average on the first level) and
// 3x3 tent upsample.

#include "../autogen/layout/DefaultLayout.h"

float bloom_luma(float3 c)
{
	return dot(c, float3(0.2126, 0.7152, 0.0722));
}

float3 tent9(Texture2D<float4> tex, float2 uv, float2 texel, float radius)
{
	float2 d = texel * radius;
	float3 s = 0;
	s += tex.SampleLevel(linearClampSampler, uv + float2(-d.x, -d.y), 0).rgb;
	s += tex.SampleLevel(linearClampSampler, uv + float2( 0,   -d.y), 0).rgb * 2;
	s += tex.SampleLevel(linearClampSampler, uv + float2( d.x, -d.y), 0).rgb;
	s += tex.SampleLevel(linearClampSampler, uv + float2(-d.x,  0),   0).rgb * 2;
	s += tex.SampleLevel(linearClampSampler, uv,                      0).rgb * 4;
	s += tex.SampleLevel(linearClampSampler, uv + float2( d.x,  0),   0).rgb * 2;
	s += tex.SampleLevel(linearClampSampler, uv + float2(-d.x,  d.y), 0).rgb;
	s += tex.SampleLevel(linearClampSampler, uv + float2( 0,    d.y), 0).rgb * 2;
	s += tex.SampleLevel(linearClampSampler, uv + float2( d.x,  d.y), 0).rgb;
	return s / 16;
}

#ifdef BUILD_FUNC_CS_Downsample
#include "../autogen/BloomDownsample.h"
#include "../autogen/FrameInfo.h"

float3 karis_group(float3 a, float3 b, float3 c, float3 d, float weight, inout float total)
{
	float3 avg = (a + b + c + d) * 0.25;
	float w = weight / (1.0 + bloom_luma(avg));
	total += w;
	return avg * w;
}

[numthreads(8, 8, 1)]
void CS_Downsample(uint3 id : SV_DispatchThreadID)
{
	const BloomDownsample data = GetBloomDownsample();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	Texture2D<float4> src = data.GetSource();
	float2 uv = (id.xy + 0.5) / float2(size);
	float2 t = data.GetSource_texel();

	// camera.jitter is half the NDC projection offset, i.e. already in uv
	// units, with NDC's +y up; uv +y is down, hence the flip. Content rendered
	// with the jittered projection sits shifted by that amount, so sampling
	// shifted by the same amount reads it back on an unjittered grid.
	if (data.GetUnjitter() != 0)
	{
		float2 j = GetFrameInfo().GetCamera().GetJitter().xy;
		uv += float2(j.x, -j.y);
	}

	// max(0): the scene can carry negative/NaN values from upstream passes,
	// and one of them would otherwise smear across the entire chain.
	#define TAP(x, y) max(src.SampleLevel(linearClampSampler, uv + t * float2(x, y), 0).rgb, 0)
	float3 a = TAP(-2, -2), b = TAP(0, -2), c = TAP(2, -2);
	float3 d = TAP(-2,  0), e = TAP(0,  0), f = TAP(2,  0);
	float3 g = TAP(-2,  2), h = TAP(0,  2), i = TAP(2,  2);
	float3 j = TAP(-1, -1), k = TAP(1, -1);
	float3 l = TAP(-1,  1), m = TAP(1,  1);
	#undef TAP

	float3 result;
	if (data.GetKaris() != 0)
	{
		float total = 0;
		result  = karis_group(j, k, l, m, 0.5,   total);
		result += karis_group(a, b, d, e, 0.125, total);
		result += karis_group(b, c, e, f, 0.125, total);
		result += karis_group(d, e, g, h, 0.125, total);
		result += karis_group(e, f, h, i, 0.125, total);
		result /= max(total, 1e-6);
	}
	else
	{
		result  = e * 0.125;
		result += (a + c + g + i) * 0.03125;
		result += (b + d + f + h) * 0.0625;
		result += (j + k + l + m) * 0.125;
	}

	float threshold = data.GetThreshold();
	if (threshold > 0)
	{
		// The buffer is uninitialized until Tonemap's first write.
		float exposure = data.GetExposure_state()[0].y;
		if (!isfinite(exposure) || exposure <= 0)
			exposure = 1;

		// Soft-knee bright pass (Unity's classic bloom prefilter): quadratic
		// ramp over [threshold - knee, threshold + knee], then linear excess.
		float brightness = max(result.r, max(result.g, result.b)) * exposure;
		float knee = threshold * data.GetKnee() + 1e-5;
		float soft = clamp(brightness - threshold + knee, 0, 2 * knee);
		soft = soft * soft / (4 * knee);
		result *= max(soft, brightness - threshold) / max(brightness, 1e-5);
	}

	target[id.xy] = float4(result, 1);
}
#endif

#ifdef BUILD_FUNC_CS_Temporal
#include "../autogen/BloomTemporal.h"
#include "../autogen/FrameInfo.h"

[numthreads(8, 8, 1)]
void CS_Temporal(uint3 id : SV_DispatchThreadID)
{
	const BloomTemporal data = GetBloomTemporal();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	Texture2D<float4> current = data.GetCurrent();
	float3 cur = current.Load(int3(id.xy, 0)).rgb;

	float3 mn = cur, mx = cur;
	[unroll]
	for (int y = -1; y <= 1; y++)
	[unroll]
	for (int x = -1; x <= 1; x++)
	{
		int2 p = clamp(int2(id.xy) + int2(x, y), 0, int2(size) - 1);
		float3 c = current.Load(int3(p, 0)).rgb;
		mn = min(mn, c);
		mx = max(mx, c);
	}

	float2 uv = (id.xy + 0.5) / float2(size);
	float2 prev_uv;
	if (data.GetDepth().SampleLevel(pointClampSampler, uv, 0) == 0)
	{
		// Sky (reversed-Z far plane) has no motion vectors. At infinity only
		// camera rotation matters: project this pixel's view direction with
		// last frame's view-projection as a direction (w = 0).
		const Camera cam = GetFrameInfo().GetCamera();
		float4 p = mul(cam.GetInvViewProj(), float4(uv * float2(2, -2) + float2(-1, 1), 1, 1));
		float3 dir = p.xyz / p.w - cam.GetPosition().xyz;
		float4 prev = mul(GetFrameInfo().GetPrevCamera().GetViewProj(), float4(dir, 0));
		prev_uv = prev.w > 0 ? (prev.xy / prev.w) * float2(0.5, -0.5) + 0.5 : float2(-1, -1);
	}
	else
	{
		// GBuffer_Speed holds prev_uv - cur_uv, unjittered.
		prev_uv = uv + data.GetMotion().SampleLevel(pointClampSampler, uv, 0);
	}

	float3 result = cur;
	if (data.GetReset() == 0 && all(prev_uv >= 0) && all(prev_uv <= 1))
	{
		float3 hist = data.GetHistory().SampleLevel(linearClampSampler, prev_uv, 0).rgb;
		if (all(isfinite(hist)))
			result = lerp(cur, clamp(hist, mn, mx), data.GetHistory_weight());
	}

	target[id.xy] = float4(result, 1);
}
#endif

#ifdef BUILD_FUNC_CS_Copy
#include "../autogen/BloomCopy.h"

[numthreads(8, 8, 1)]
void CS_Copy(uint3 id : SV_DispatchThreadID)
{
	const BloomCopy data = GetBloomCopy();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	target[id.xy] = data.GetSource().Load(int3(id.xy, 0));
}
#endif

#ifdef BUILD_FUNC_CS_Upsample
#include "../autogen/BloomUpsample.h"

[numthreads(8, 8, 1)]
void CS_Upsample(uint3 id : SV_DispatchThreadID)
{
	const BloomUpsample data = GetBloomUpsample();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	float2 uv = (id.xy + 0.5) / float2(size);
	float3 up = tent9(data.GetLow(), uv, data.GetLow_texel(), data.GetRadius());
	float3 base = data.GetHigh().Load(int3(id.xy, 0)).rgb;

	float3 result = data.GetMode() == BloomMode::Scatter ? lerp(base, up, data.GetScatter()) : base + up;
	target[id.xy] = float4(result, 1);
}
#endif

#ifdef BUILD_FUNC_CS_Composite
#include "../autogen/BloomComposite.h"

[numthreads(8, 8, 1)]
void CS_Composite(uint3 id : SV_DispatchThreadID)
{
	const BloomComposite data = GetBloomComposite();
	RWTexture2D<float4> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	float2 uv = (id.xy + 0.5) / float2(size);
	float4 scene = data.GetScene().Load(int3(id.xy, 0));
	float3 bloom = tent9(data.GetBloom(), uv, data.GetBloom_texel(), data.GetRadius()) * data.GetBloom_scale();

	float3 result = data.GetAdditive() != 0
		? scene.rgb + bloom * data.GetIntensity()
		: lerp(scene.rgb, bloom, data.GetIntensity());

	if (data.GetUse_ghosts() != 0)
		result += data.GetFlare_ghosts().SampleLevel(linearClampSampler, uv, 0).rgb;
	if (data.GetUse_streaks() != 0)
		result += data.GetFlare_streaks().SampleLevel(linearClampSampler, uv, 0).rgb * data.GetStreak_intensity();

	target[id.xy] = float4(result, scene.a);
}
#endif
