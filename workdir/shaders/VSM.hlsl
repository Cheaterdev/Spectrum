#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMConstants.h"
// Only actually referenced inside get_shadow_vsm's VSM_RTX_VERIFY branch;
// unconditionally included since it's dead-code-eliminated when the define
// is off, same as every other permutation-gated accessor in this codebase.
#include "autogen/Raytracing.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

#include "PBR.hlsl"
#include "VSM_impl.hlsl"

float2 GetBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, NoV), 0);
}

float4 combine_result(float2 tc, uint2 pixel)
{
	pixel_info info;
	Camera camera = GetFrameInfo().GetCamera();
	float4 packed_0 = gbuffer.GetAlbedo().SampleLevel(pointClampSampler, tc, 0);

	info.albedo = packed_0.rgb;
	info.metallic = packed_0.w;

	info.normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	// Sky/background pixels (no geometry) skip shading entirely. This used
	// to need a has_geometry/no-early-return workaround instead of a plain
	// early return: get_shadow_vsm's blocker search ran quad-shared
	// (QuadReadAcrossX/Y/Diagonal) inline in THIS dispatch, and a
	// compute-shader `return` genuinely deactivates a thread (no
	// pixel-shader helper-invocation guarantee), so an early return here
	// would've undefined a still-active quad-mate's reads. The blocker
	// search extraction moved all quad ops into VSM_BlockerSearch's own
	// dispatch (VSM_BlockerSearch.hlsl) -- get_shadow_vsm no longer does
	// any quad ops itself, so a plain early return is safe again here.
	if (raw_z == 0) return 0;
	info.pos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());

	info.roughness = max(0.04, gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).w);
	info.view = normalize(camera.GetPosition() - info.pos);

	VSMConstants constants = GetVSMConstants();
	float shadow = get_shadow_vsm(constants, GetVSMLighting(), info.pos, info.normal, pixel);

	// Debug view (runtime toggle, VSM.ixx's use_vsm_debug_rtx_reference):
	// bypass VSM's own shadow entirely and show RTXShadow's own denoised
	// full-RT shadow mask as grayscale.
	if (constants.GetDebug_rtx_reference() != 0)
	{
		float rtx_shadow = GetVSMLighting().GetRtx_shadow_mask().SampleLevel(pointClampSampler, tc, 0);
		return float4(rtx_shadow, rtx_shadow, rtx_shadow, 1);
	}
//	#define VSM_DEBUG_HEATMAP
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
	// Blocker-search extraction moved quad ops entirely into
	// VSM_BlockerSearch's own dispatch -- an early return for out-of-bounds
	// padding threads is safe again here (see combine_result's own comment
	// on the matching sky-pixel case).
	if (any(DTid.xy >= dims))
		return;

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	GetVSMLighting().GetResult()[DTid.xy] = combine_result(tc, DTid.xy);
}
