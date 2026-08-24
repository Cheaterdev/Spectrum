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

	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	// Was an early `return 0` here for sky/background pixels (raw_z==0, no
	// geometry) -- removed. get_shadow_vsm's quad-shared blocker search
	// (VSM_impl.hlsl, quad_blocker_search) needs every thread in a 2x2
	// dispatch quad to reach its QuadReadAcrossX/Y/Diagonal calls
	// uniformly, and this early return fired at EVERY silhouette edge in
	// the whole image -- by far the single biggest source of the
	// divergence artifacts quad-sharing otherwise produces (much more
	// common than CS_RESULT's screen-edge bounds check). has_geometry
	// gates the final returned color instead; info.pos/info.normal fall
	// back to safe, finite dummy values for sky pixels (camera.GetPosition()
	// / a fixed up vector) rather than whatever depth_to_wpos(0, ...) or an
	// unwritten gbuffer normal texel would produce, so get_shadow_vsm never
	// sees NaN/Inf-risking input even though its result is discarded here.
	bool has_geometry = (raw_z != 0);
	info.normal = has_geometry
		? normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1)
		: float3(0, 1, 0);
	info.pos = has_geometry ? depth_to_wpos(raw_z, tc, camera.GetInvViewProj()) : camera.GetPosition();

	info.roughness = max(0.04, gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).w);
	info.view = normalize(camera.GetPosition() - info.pos);

	VSMConstants constants = GetVSMConstants();
	float shadow = get_shadow_vsm(constants, GetVSMLighting(), info.pos, info.normal, pixel);
	if (!has_geometry)
		return 0;
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
	// Was an early `return` here for padding threads past the dispatch's
	// clamped-to-16 screen bounds -- removed for the same reason as
	// combine_result's raw_z==0 removal (see its comment): a compute-shader
	// `return` deactivates the thread before it can reach get_shadow_vsm's
	// quad-shared blocker search, undefining its still-active quad-mates'
	// QuadReadAcrossX/Y/Diagonal reads right along the screen edge. tc
	// beyond [0,1] just clamps to the edge texel (pointClampSampler) --
	// harmless, redundant work for padding threads, discarded by the
	// in-bounds check on the write below instead of skipping the compute.
	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	float4 result = combine_result(tc, DTid.xy);
	if (all(DTid.xy < dims))
		GetVSMLighting().GetResult()[DTid.xy] = result;
}
