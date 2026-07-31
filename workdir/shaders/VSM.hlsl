#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMConstants.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

#include "PBR.hlsl"
#include "VSM_impl.hlsl"

float2 GetBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, NoV), 0);
}

float4 combine_result(float2 tc)
{
	pixel_info info;
	Camera camera = GetFrameInfo().GetCamera();
	float4 packed_0 = gbuffer.GetAlbedo().SampleLevel(pointClampSampler, tc, 0);

	info.albedo = packed_0.rgb;
	info.metallic = packed_0.w;

	info.normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	if (raw_z == 0) return 0;
	info.pos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());

	info.roughness = max(0.04, gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).w);
	info.view = normalize(camera.GetPosition() - info.pos);

	VSMConstants constants = GetVSMConstants();
	float shadow = get_shadow_vsm(constants, GetVSMLighting(), info.pos);
	//#define VSM_DEBUG_HEATMAP
//	#define VSM_DEBUG_RAWDEPTH
#ifdef VSM_DEBUG_HEATMAP
	return float4(get_vsm_debug_color(constants, info.pos) * shadow, 1);
#endif
#ifdef VSM_DEBUG_RAWDEPTH
	// Grayscale = raw sampled atlas depth (reversed-Z, clear=0). All-black
	// means nothing was ever rasterized into the atlas at that page.
	float raw_depth = get_vsm_debug_raw_depth(constants, GetVSMLighting(), info.pos);
	return float4(raw_depth, raw_depth, raw_depth, 1);
#endif

	float3 light_dir = normalize(GetFrameInfo().GetSunDir().xyz);
	float NV = saturate(dot(info.normal, info.view));
	float NL = saturate(dot(info.normal, light_dir));

	float2 EnvBRDF = GetBRDF(info.roughness, 0, 0.5 + 0.5 * NL) * GetBRDF(info.roughness, 0, 0.5 + 0.5 * NV);

    return float4(shadow * (NL * info.albedo * (1 - info.metallic)), 1);
}

// Compute-queue variant: one thread per output pixel, writes the result UAV.
[numthreads(16, 16, 1)]
void CS_RESULT(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	GetVSMLighting().GetResult().GetDimensions(dims.x, dims.y);
	if (any(DTid.xy >= dims))
		return;

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	GetVSMLighting().GetResult()[DTid.xy] = combine_result(tc);
}
