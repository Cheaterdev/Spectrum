#define M_PI 3.141592653589793238462643383279f

#include "../2D_screen_simple.h"

#ifdef BUILD_FUNC_PS

#include "../autogen/Countour.h"

static const Countour contour = GetCountour();
static const Texture2D<uint> object_ids = contour.GetObject_ids();

// ResultTexture may be upscaled relative to the GBuffer, so texels are
// addressed through tc rather than SV_Position.
float selected_at(float2 tc, int2 offset, int2 dims)
{
	int2 p = clamp(int2(tc * dims) + offset, 0, dims - 1);
	return object_ids.Load(int3(p, 0)) == contour.GetSelected_id() ? 1 : 0;
}

float4 PS(quad_output input) : SV_TARGET0
{
	int2 dims;
	object_ids.GetDimensions(dims.x, dims.y);

	float orig = selected_at(input.tc, 0, dims);

	float2 res = 0;
	[unroll] for (int i = -2; i <= 2; i++)
	[unroll] for (int j = -2; j <= 2; j++)
	{
		float w = 1 - length(float2(i,j)) / length(float2(2,2));
		res += float2(selected_at(input.tc, int2(i,j), dims) * w, w);
	}

	return float4(saturate((res.x / res.y - orig * 0.99).xxx) * contour.GetColor().xyz, 1);
}

#endif
