#define M_PI 3.141592653589793238462643383279f

#include "2D_screen_simple.h"

#ifdef BUILD_FUNC_PS

#include "autogen/Countour.h"

float4 PS(quad_output input) : SV_TARGET0
{
	float orig = GetCountour().GetTex().Sample(pointClampSampler, input.tc);

	float2 res = 0;
	[unroll] for (int i = -2; i <= 2; i++)
	[unroll] for (int j = -2; j <= 2; j++)
	{
		float w = 1 - length(float2(i,j)) / length(float2(2,2));
		res += float2(GetCountour().GetTex().Sample(pointClampSampler, input.tc ,int2(i,j)).x * w,w);
	}

	return  float4(saturate((res.x / res.y - orig * 0.99).xxx) * GetCountour().GetColor().xyz,1);
}

#endif