#define SIZE 16
#define FIX 8

#include "autogen/FrameInfo.h"
#include "autogen/DenoiserHistoryFix.h"

static const Camera camera = GetFrameInfo().GetCamera();

static const DenoiserHistoryFix denoiser_history = GetDenoiserHistoryFix();

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


#define MOVE_SCALER 0.1

static const Texture2D<float4> color_tex = denoiser_history.GetColor();
static const Texture2D<float> tex_frames = denoiser_history.GetFrames();

static const RWTexture2D<float4> target = denoiser_history.GetTarget();

static const int2 delta[4] =
{
    int2(0, 1),
    int2(1, 1),
    int2(1, 0),
    int2(0, 0)
};

static const int2 offset[4] =
{
	int2(0, 0),
	int2(0, 1),
	int2(1, 0),
	int2(1, 1)
};


struct Bilinear { float2 origin; float2 weights; };
Bilinear GetBilinearFilter(float2 uv, float2 texSize)
{
	Bilinear result;
	result.origin = floor(uv * texSize - 0.5);
	result.weights = frac(uv * texSize - 0.5);
	return result;
}
float4 GetBilinearCustomWeights(Bilinear f, float4 customWeights)
{
	float4 weights;
	weights.x = (1.0 - f.weights.x) * (1.0 - f.weights.y);
	weights.y = f.weights.x * (1.0 - f.weights.y);
	weights.z = (1.0 - f.weights.x) * f.weights.y;
	weights.w = f.weights.x * f.weights.y;
	return weights * customWeights;
}
float4 ApplyBilinearCustomWeights(float4 s00, float4 s10, float4 s01, float4 s11, float4 w, bool normalize = true)
{
	float4 r = s00 * w.x + s10 * w.y + s01 * w.z + s11 * w.w;
	return r * (normalize ? rcp(dot(w, 1.0)) : 1.0);
}



float get_occlusion(float prev_z, float2 tc, float3 pos)
{
	float3 prev_pos = depth_to_wpos(prev_z, tc, camera.GetInvViewProj());

	float l = 5*length(pos - prev_pos) * MOVE_SCALER;

	return 0.0005+ all(tc > 0 && tc < 1)*saturate(1 - l);
	//return all(tc > 0 && tc < 1) && l < 0.1 ? 1 : 0.005;
}

[numthreads(8, 8, 1)]
void  CS(uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{

	uint2 index = denoiser_history.GetTiling().get_pixel_pos(dispatchID);


	
	float2 targetDims;

	target.GetDimensions(targetDims.x, targetDims.y);
    float2 dims;
	color_tex.GetDimensions(dims.x, dims.y);
	
	float2 tc = index / targetDims;
	float normAccumulatedFrameNum = tex_frames.SampleLevel(pointBorderSampler, tc, 0);
	float4 result = color_tex.SampleLevel(pointBorderSampler, tc, 0);

	float3 pos = depth_to_wpos(result.w, tc, camera.GetInvViewProj());

	//if (normAccumulatedFrameNum >= 1 - 0.5/8)
	//	discard;
	float roughness = 1;
	uint mipLevel =  4.0 * (1.0 - normAccumulatedFrameNum) * roughness;

	if (mipLevel == 0)
		return;
	
	float2 mipSize = dims / (1 << mipLevel);
	Bilinear filter = GetBilinearFilter(tc, mipSize);
	float2 gatherUv = (float2(filter.origin) + 0.5) / mipSize;
	
	float4 s00, s10, s01, s11;
	
	s00 = color_tex.SampleLevel(pointBorderSampler, gatherUv + float2(offset[0]) / mipSize, mipLevel );
	s01 = color_tex.SampleLevel(pointBorderSampler, gatherUv + float2(offset[1]) / mipSize, mipLevel );
	s10 = color_tex.SampleLevel(pointBorderSampler, gatherUv + float2(offset[2]) / mipSize, mipLevel );
	s11 = color_tex.SampleLevel(pointBorderSampler, gatherUv + float2(offset[3]) / mipSize, mipLevel );

	float4 occlusion = 0;


	occlusion.x = get_occlusion(s00.w, gatherUv + float2(offset[0]) / mipSize, pos);
	occlusion.y = get_occlusion(s01.w, gatherUv + float2(offset[1]) / mipSize, pos);
	occlusion.z = get_occlusion(s10.w, gatherUv + float2(offset[2]) / mipSize, pos);
	occlusion.w = get_occlusion(s11.w, gatherUv + float2(offset[3]) / mipSize, pos);

	
	float4 bilateralWeights = GetBilinearCustomWeights(filter, occlusion);
	float4 bilinearWeights = GetBilinearCustomWeights(filter, 1.0);

	float4 w = bilateralWeights;
	
	float4 blurry = ApplyBilinearCustomWeights(s00, s10, s01, s11, w, true);


	target[index] = blurry;
}

