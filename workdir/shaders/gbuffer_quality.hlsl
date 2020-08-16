#include "2D_screen_simple.h"

#include "autogen/FrameInfo.h"
static const Camera camera = GetFrameInfo().GetCamera();

#ifdef BUILD_FUNC_PS
#include "autogen/GBuffer.h"
static const GBuffer gbuffer = GetGBuffer();

float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc, d, 1));
	return P.xyz / P.w;
}

float2 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return res.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
}


static const int2 delta2[4] =
{
	int2(0, 1),
	int2(1, 1),
	int2(1, 0),
	int2(0, 0),

};


float4 PS(quad_output i) :SV_Target0
{
	float2 dims;
	gbuffer.GetNormals().GetDimensions(dims.x, dims.y);

	float3 low_normals = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, i.tc, 1).xyz * 2 - 1);
	float low_depth = gbuffer.GetDepth().SampleLevel(pointClampSampler, i.tc, 1);

	float3 p2 = depth_to_wpos(low_depth, i.tc.xy, camera.GetInvViewProj());


	float result_color = 1;
	float result_reflection = 1;

	float2 full_tc = i.tc - float2(0.5, 0.5) / dims;

	float3 v = normalize(p2 - camera.GetPosition());
	float3 r = normalize(reflect(v, low_normals));

#define R 1

	[unroll] for (int x = -R; x <= R; x++)
	[unroll] for (int y = -R; y <= R; y++)
	{
		//float2 t_tc = i.tc + 2 * float2(x, y) / dims;
		int2 deltas = int2(x, y);

		float3 full_normals = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, full_tc, 0, deltas).xyz * 2 - 1);
		float full_depth = gbuffer.GetDepth().SampleLevel(pointClampSampler, full_tc, 0, deltas);



		float3 p1 = depth_to_wpos(full_depth, i.tc.xy, camera.GetInvViewProj());
		float3 delta = p1 - p2;


		float3 _v = normalize(p1 - camera.GetPosition());
		float3 _r = normalize(reflect(_v, full_normals));



		float my_result = 1;
		float pos_w = length(delta) < (length(p2 - camera.GetPosition()) * 0.02);
		float normal_w = pow(saturate(dot(full_normals, low_normals)), 32);
		float refl_w = pow(saturate(dot(r, _r)), 256);



		result_color = min(result_color, pos_w * normal_w);
		result_reflection = min(result_reflection, pos_w * refl_w);

	}

	return float4(result_color, result_reflection,0,0);
}

#else
#include "autogen/GBufferQuality.h"
void PS_STENCIL(quad_output i)
{
	clip(0.05 - GetGBufferQuality().GetRef().SampleLevel(pointClampSampler, i.tc, 0).x);
}

void PS_STENCIL_REFL(quad_output i)
{
	//discard;
	clip(0.05 - GetGBufferQuality().GetRef().SampleLevel(pointClampSampler, i.tc, 0).y);
}


#endif
