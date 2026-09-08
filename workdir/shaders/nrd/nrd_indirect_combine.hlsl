#include "../common/common.hlsl"

#include "../autogen/FrameInfo.h"
#include "../autogen/NRD_IndirectCombineParams.h"

// REBLUR_BackEnd_UnpackRadianceAndNormHitDist, to decode the REBLUR-denoised
// indirect signal below (see [[project-nrd-integration]]) -- self-contained,
// same as rtx_combine.hlsl's own #include of this file.
#include "3rdparty/NRD.hlsli"

static const GBuffer gbuffer = GetNRD_IndirectCombineParams().GetGbuffer();

float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, 0.5 + 0.5 * NoV), 0);
}

// This is the FSR/DLSS-side equivalent of RTXCombine's indirect term, so it
// adds onto ResultTexture the same way VoxelCombine's own blur pass
// (voxel_screen_blur.hlsl) does for its own indirect term -- same formula,
// kept identical so swapping the source (REBLUR-denoised vs the legacy
// spatially-blurred signal) doesn't change the look on its own.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	float2 dims;
	GetNRD_IndirectCombineParams().GetTarget().GetDimensions(dims.x, dims.y);

	int2 tc = dispatchID.xy;
	if (any(tc >= int2(dims))) return;

	// No geometry -- nothing to shade, same reversed-Z sky check every other
	// GBuffer-driven pass in this file uses.
	float raw_z = gbuffer.GetDepth()[tc];
	if (raw_z == 0)
		return;

	float4 albedo = gbuffer.GetAlbedo()[tc];
	float roughness = pow(max(MIN_ROUGHNESS, gbuffer.GetNormals()[tc].w), 2);
	float metallic = albedo.w;

	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);
	float2 itc = (float2(tc) + 0.5) / dims;
	float3 pos = depth_to_wpos(raw_z, itc, GetFrameInfo().GetCamera().GetInvViewProj());
	float3 v = normalize(pos - GetFrameInfo().GetCamera().GetPosition());

	float3 indirect = REBLUR_BackEnd_UnpackRadianceAndNormHitDist(GetNRD_IndirectCombineParams().GetIndirect()[tc]).rgb;

	float NoV = dot(normal, v);
	float3 color = albedo.rgb * IntegrateBRDF(roughness, metallic, -NoV).x * indirect;

	GetNRD_IndirectCombineParams().GetTarget()[tc] += float4(color, 1);
}
