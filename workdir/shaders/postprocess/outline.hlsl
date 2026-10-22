// Selection outline from the supersampled mask, accumulated over frames (see
// OutlineData in stenciler.prism).

#include "../autogen/layout/DefaultLayout.h"

#ifdef BUILD_FUNC_CS_Outline
#include "../autogen/OutlineData.h"
#include "../autogen/FrameInfo.h"

// Mask alpha encoding (DrawMask / PS_MASK): fract = visibility * 0.5; the
// integer part is the gizmo.
float mask_at(Texture2D<float4> src, int2 p, int2 dims)
{
	p = clamp(p, 0, dims - 1);
	return saturate(frac(src.Load(int3(p, 0)).a) * 2);
}

float outline_edge(Texture2D<float4> src, int2 id, float width)
{
	int2 dims;
	src.GetDimensions(dims.x, dims.y);
	const int2 p = id * 2;

	float orig = 0;
	[unroll] for (int y = 0; y < 2; y++)
	[unroll] for (int x = 0; x < 2; x++)
		orig += mask_at(src, p + int2(x, y), dims);
	orig *= 0.25;

	const float reach = width * 1.41421356f;
	const int   taps  = (int)ceil(reach * 2);
	const int2  c     = p + 1;

	// Early out away from the mask boundary: a sparse grid over the kernel
	// that is entirely inside or entirely outside has no edge to draw.
	float sparse = 0;
	[unroll] for (int sy = -2; sy <= 2; sy++)
	[unroll] for (int sx = -2; sx <= 2; sx++)
		sparse += mask_at(src, c + int2(sx, sy) * max(taps / 2, 1), dims);

	if (sparse <= 0.01 || sparse >= 24.99)
		return 0;

	float2 res = 0;
	[loop] for (int j = -taps; j <= taps; j++)
	[loop] for (int i = -taps; i <= taps; i++)
	{
		float w = 1 - length(float2(i, j) * 0.5) / reach;
		if (w <= 0)
			continue;
		res += float2(mask_at(src, c + int2(i, j), dims) * w, w);
	}

	// The raw value peaks around 0.5 right at the boundary; x2 so the line
	// reaches full strength there.
	return saturate((res.x / res.y - orig * 0.99) * 2);
}

[numthreads(8, 8, 1)]
void CS_Outline(uint3 id : SV_DispatchThreadID)
{
	const OutlineData data = GetOutlineData();
	RWTexture2D<float> target = data.GetEdge();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	const float cur = outline_edge(data.GetMask(), int2(id.xy), data.GetWidth());

	// This frame's values around the pixel, to clamp history to: the crawl of
	// the occluder edges moves the line by up to a texel either way.
	float mn = cur, mx = cur;
	[unroll] for (int y = -1; y <= 1; y++)
	[unroll] for (int x = -1; x <= 1; x++)
	{
		int2 q = clamp(int2(id.xy) + int2(x, y), 0, int2(size) - 1);
		float v = outline_edge(data.GetMask(), q, data.GetWidth());
		mn = min(mn, v);
		mx = max(mx, v);
	}

	float2 uv = (id.xy + 0.5) / float2(size);
	float2 prev_uv;
	if (data.GetDepth().SampleLevel(pointClampSampler, uv, 0) == 0)
	{
		// Sky has no motion vectors: only camera rotation matters at
		// infinity (same as the bloom resolve).
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

	float result = cur;
	if (data.GetReset() == 0 && all(prev_uv >= 0) && all(prev_uv <= 1))
	{
		float hist = data.GetHistory().SampleLevel(linearClampSampler, prev_uv, 0);
		if (isfinite(hist))
			result = lerp(cur, clamp(hist, mn, mx), data.GetHistory_weight());
	}

	target[id.xy] = result;
}
#endif

#ifdef BUILD_FUNC_CS_Copy
#include "../autogen/OutlineCopy.h"

[numthreads(8, 8, 1)]
void CS_Copy(uint3 id : SV_DispatchThreadID)
{
	const OutlineCopy data = GetOutlineCopy();
	RWTexture2D<float> target = data.GetTarget();

	uint2 size;
	target.GetDimensions(size.x, size.y);
	if (any(id.xy >= size))
		return;

	target[id.xy] = data.GetSource().Load(int3(id.xy, 0));
}
#endif
