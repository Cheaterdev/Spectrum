#include "../2D_screen_simple.h"

#ifdef BUILD_FUNC_PS

#include "../autogen/GizmoResolveData.h"

// The target is 2x the output in each axis. Averages each 2x2 block for the
// gizmo (alpha >= 1; the fraction is the selection mask, see outline.hlsl) and
// composites it over the accumulated selection outline.
float4 PS(quad_output input) : SV_TARGET0
{
	const GizmoResolveData data = GetGizmoResolveData();
	Texture2D<float4> src = data.GetSource();
	int2 p = int2(input.pos.xy) * 2;

	float3 gizmo = 0;
	float  ga = 0;
	[unroll] for (int y = 0; y < 2; y++)
	[unroll] for (int x = 0; x < 2; x++)
	{
		float4 t = src.Load(int3(p + int2(x, y), 0));
		if (t.a >= 0.75)
		{
			gizmo += t.rgb;
			ga += 1;
		}
	}
	gizmo *= 0.25;
	ga *= 0.25;

	float edge = data.GetEdge().Load(int3(input.pos.xy, 0));

	// gizmo (already coverage-weighted) over outline, for AlphaBlend onto the frame.
	// outline_color.a is an opacity gain on the (soft, antialiased) edge strength.
	float  oa = saturate(edge * data.GetOutline_color().a);
	float3 premult = gizmo + (1 - ga) * oa * data.GetOutline_color().rgb;
	float  a = ga + (1 - ga) * oa;
	if (a <= 0)
		return 0;
	return float4(premult / a, a);
}

#endif
