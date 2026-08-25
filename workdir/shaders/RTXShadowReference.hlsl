#include "Common.hlsl"

#include "autogen/RTXShadowReference.h"
#include "autogen/FrameInfo.h"
#include "autogen/Raytracing.h"

// Deterministic per-pixel angle -- no blue noise texture dependency here
// (this file is deliberately self-contained, independent of VSM's own
// code/bindings, so the comparison it's meant to enable stays honest).
// Same hash shape used elsewhere in this codebase for the same purpose.
float rtx_reference_hash_angle(uint2 p)
{
	uint h = p.x * 374761393u + p.y * 668265263u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h = h ^ (h >> 16);
	return (h & 0xFFFFu) / 65536.0 * 6.28318530718;
}

float2 rtx_reference_rotate(float2 v, float angle)
{
	float s, c;
	sincos(angle, s, c);
	return float2(v.x * c - v.y * s, v.x * s + v.y * c);
}

static const float2 RTX_REFERENCE_POISSON_DISK[16] = {
	float2(-0.94201624, -0.39906216), float2( 0.94558609, -0.76890725),
	float2(-0.09418410, -0.92938870), float2( 0.34495938,  0.29387760),
	float2(-0.91588581,  0.45771432), float2(-0.81544232, -0.87912464),
	float2(-0.38277543,  0.27676845), float2( 0.97484398,  0.75648379),
	float2( 0.44323325, -0.97511554), float2( 0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023), float2( 0.79197514,  0.19090188),
	float2(-0.24188840,  0.99706507), float2(-0.81409955,  0.91437590),
	float2( 0.19984126,  0.78641367), float2( 0.14383161, -0.14100790),
};

// RTXShadow's debug reference mode (see RTX::debug_full_reference_shadow in
// RTX.ixx, and PassDefaults.cpp's RTXShadow::render for how this gets
// selected instead of the pass's normal Bend/FFX hybrid-shadow-denoiser
// dispatch). 16 real rays sampling the sun's full angular disc per pixel --
// genuine stochastic soft shadow, no denoiser -- a ground truth to compare
// VSM's own PCSS approximation against. Deliberately has NO dependency on
// any VSM code/types (separate binding struct, separate Poisson disc,
// separate hash function) -- it needs to be an independent reference, not
// something that could accidentally inherit a bug VSM's own path has.
[numthreads(16, 16, 1)]
void CS_REFERENCE(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	GetRTXShadowReference().GetOutput().GetDimensions(dims.x, dims.y);
	if (any(DTid.xy >= dims))
		return;

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	GBuffer gbuffer = GetRTXShadowReference().GetGbuffer();
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	if (raw_z == 0)
	{
		// Sky/background -- nothing to shadow, treat as fully lit.
		GetRTXShadowReference().GetOutput()[DTid.xy] = float4(1, 1, 1, 1);
		return;
	}

	Camera camera = GetFrameInfo().GetCamera();
	float3 wpos   = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1);

	// Matches VSM_impl.hlsl's own VSM_SUN_ANGULAR_RADIUS by value, not by
	// sharing the constant -- this file is deliberately independent (see
	// its own top comment), so keep this in sync by hand if that value
	// ever changes and the comparison should track it.
	static const float RTX_REFERENCE_SUN_ANGULAR_RADIUS = 0.02; // ~17 deg.

	float3 sun_dir = normalize(GetFrameInfo().GetSunDir().xyz);
	float3 up      = (abs(sun_dir.y) > 0.99) ? float3(1, 0, 0) : float3(0, 1, 0);
	float3 right   = normalize(cross(up, sun_dir));
	up             = normalize(cross(sun_dir, right));
	float3 ray_origin = wpos + normal * 0.005;
	float  rotate_angle = rtx_reference_hash_angle(DTid.xy);

	int lit_count = 0;
	[unroll]
	for (int i = 0; i < 16; i++)
	{
		float2 offset = rtx_reference_rotate(RTX_REFERENCE_POISSON_DISK[i], rotate_angle);
		float3 dir = normalize(sun_dir +
			(right * offset.x + up * offset.y) * tan(RTX_REFERENCE_SUN_ANGULAR_RADIUS));

		RayDesc ray;
		ray.Origin    = ray_origin;
		ray.Direction = dir;
		ray.TMin      = 0.01;
		ray.TMax      = 500.0;

		// ACCEPT_FIRST_HIT_AND_END_SEARCH is safe here -- pure hit/miss
		// test, not a distance measurement, so the first hit found is
		// exactly as good as the closest one for deciding occluded-vs-not.
		RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
		rayQuery.TraceRayInline(GetRaytracing().GetScene(), RAY_FLAG_NONE, 0xFF, ray);
		rayQuery.Proceed();

		if (rayQuery.CommittedStatus() == COMMITTED_NOTHING)
			lit_count++;
	}

	float shadow = lit_count / 16.0;
	GetRTXShadowReference().GetOutput()[DTid.xy] = float4(shadow, shadow, shadow, 1);
}
