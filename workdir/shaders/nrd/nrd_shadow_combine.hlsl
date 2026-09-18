#include "../common/common.hlsl"

#include "../autogen/FrameInfo.h"
#include "../autogen/NRD_ShadowCombineParams.h"

// SIGMA_BackEnd_UnpackShadow, to decode the SIGMA-denoised RTX-reference
// shadow below (see [[project-nrd-integration]]) -- self-contained, same as
// nrd_indirect_combine.hlsl's own #include of this file.
#include "3rdparty/NRD.hlsli"

static const GBuffer gbuffer = GetNRD_ShadowCombineParams().GetGbuffer();

// OVERWRITES ResultTexture's direct-lighting term (not += like
// nrd_indirect_combine.hlsl) -- this redoes the same term VSM's own
// vsm_resolve_combine (vsm_shadow_resolve.hlsl) would otherwise have
// written; exactly one of the two runs each frame (see
// VSMSelectors::shadow_source, [[project-nrd-integration]]). Same PBR
// formula as vsm_resolve_combine -- shadow * NL * albedo * (1-metallic).
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	float2 dims;
	GetNRD_ShadowCombineParams().GetTarget().GetDimensions(dims.x, dims.y);

	int2 tc = dispatchID.xy;
	if (any(tc >= int2(dims))) return;

	// No geometry -- nothing to shade, same reversed-Z sky check every other
	// GBuffer-driven pass in this file uses.
	float raw_z = gbuffer.GetDepth()[tc];
	if (raw_z == 0)
		return;

	float4 albedo = gbuffer.GetAlbedo()[tc];
	float metallic = albedo.w;
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	float shadow = SIGMA_BackEnd_UnpackShadow(GetNRD_ShadowCombineParams().GetShadow_denoised()[tc]);

	float3 light_dir = normalize(GetFrameInfo().GetSunDir().xyz);
	float  NL = saturate(dot(normal, light_dir));

	GetNRD_ShadowCombineParams().GetTarget()[tc] = float4(shadow * (NL * albedo.rgb * (1 - metallic)), 1);
}
