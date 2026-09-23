#include "../common/common.hlsl"

#include "../autogen/FrameInfo.h"
#include "../autogen/RTXCombine.h"

static const Camera camera = GetFrameInfo().GetCamera();
static const GBuffer gbuffer = GetRTXCombine().GetGbuffer();

float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, 0.5 + 0.5 * NoV), 0);
}

// Same formula ReflectionCombine (reflection_combine.hlsl) uses -- kept
// identical so switching between ReflCombine/RTXCombine doesn't change how
// reflections look, only where the noisy input comes from.
float3 get_PBR(float3 SpecularColor, float3 ReflectionColor, float3 N, float3 V, float Roughness, float Metallic)
{
	V *= -1;
	float NoV = dot(N, V);
	float2 EnvBRDF = IntegrateBRDF(Roughness, Metallic, NoV);
	return ReflectionColor * (Metallic * SpecularColor * EnvBRDF.x + EnvBRDF.y);
}

// Same unprojection GenerateCameraRay (raytracing.hlsl) uses, minus the
// depth read -- gives a valid view-ray direction for sky pixels, which have
// no depth to reconstruct a world position from.
float3 screen_ray_dir(float2 tc)
{
	float2 screenPos = tc * 2.0 - 1.0;
	screenPos.y = -screenPos.y;
	float4 world = mul(camera.GetInvViewProj(), float4(screenPos, 0, 1));
	world.xyz /= world.w;
	return normalize(world.xyz - camera.GetPosition());
}

[numthreads(8,8,1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	float2 dims;
	GetRTXCombine().GetTarget().GetDimensions(dims.x, dims.y);

	int2 tc = dispatchID.xy;
	if (any(tc >= int2(dims))) return;

	float2 itc = float2(tc + 0.5) / dims;

	float raw_z = gbuffer.GetDepth()[tc];
	if (raw_z == 0)
	{
		// No geometry -- same sky sample MyMissShader/get_history's miss
		// path use, so the background isn't just black.
		float3 sky = GetFrameInfo().GetSky().SampleLevel(linearSampler, screen_ray_dir(itc), 0);
		GetRTXCombine().GetTarget()[tc] = float4(sky, 1);
		return;
	}

	float4 albedo = gbuffer.GetAlbedo()[tc];
	float roughness = pow(max(MIN_ROUGHNESS, gbuffer.GetNormals()[tc].w), 2);
	float metallic = albedo.w;
	float3 glow = gbuffer.GetSpecular()[tc].rgb;

	float3 pos = depth_to_wpos(raw_z, itc, camera.GetInvViewProj());
	float3 v = normalize(pos - camera.GetPosition());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	// Direct sun lighting: identical formula to every RTX hit point in the
	// engine (MyClosestHitShader, UniversalMaterialRaytracing.hlsl) --
	// albedo * NdotL * sun_visibility, no separate sun color/intensity (the
	// engine's convention is a unit-intensity sun everywhere). RTXShadowNoise
	// stands in for that hit shader's own traced sun_vis.
	float3 sun_dir = normalize(GetFrameInfo().GetSunDir().xyz);
	float NdotL = saturate(dot(normal, sun_dir));
	float shadow = GetRTXCombine().GetShadow()[tc].r;
	float3 direct = albedo.rgb * NdotL * shadow * (1 - metallic);

	// Reflections: identical weighting to ReflectionCombine. Raw RTX trace
	// output now (RGB=hit color, A=hit distance, not REBLUR-packed) -- NRD
	// doesn't run under DLSS-RR any more, DLSS-RR does its own
	// reconstruction/denoising on this exact signal (see RTXCombine's own
	// comment, voxel.prism).
	float3 reflection = GetRTXCombine().GetReflection()[tc].rgb;
	float3 refl_color = get_PBR(albedo.rgb, reflection, normal, v, roughness, metallic);

	// Indirect GI: diffuse bounce light, weighted by the surface's own
	// albedo (diffuse response) rather than the specular BRDF above, and
	// rolled off by metallic (metals have ~no diffuse term). Same raw shape
	// as reflection above.
	float3 indirect = GetRTXCombine().GetIndirect()[tc].rgb;
	float3 gi_color = albedo.rgb * indirect * (1 - metallic);

	// Self-emission: not shadowed, not weighted by any BRDF -- a glowing
	// surface looks the same lit or unlit. Indirect GI/reflection rays get
	// this for free via MyClosestHitShader's own payload.color (see its
	// comment) -- this is only needed here for direct visibility, since this
	// pass reads the GBuffer rather than recomputing the material.
	GetRTXCombine().GetTarget()[tc] = float4(direct + refl_color + gi_color + glow, 1);
}
