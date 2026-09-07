//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL


#include "../autogen/FrameInfo.h"
#include "../autogen/Raytracing.h"
//#include "../autogen/RaytracingRays.h"

#include "../autogen/tables/RayPayload.h"
#include "../autogen/tables/ShadowPayload.h"
#include "../autogen/tables/ColorShadowPayload.h"
#include "../autogen/VoxelScreen.h"
#include "../autogen/VoxelInfo.h"


#include "../autogen/VoxelOutput.h"
#include "../autogen/rtx/ShadowPass.h"
#include "../autogen/rtx/ColorPass.h"
#include "../common/pbr.hlsl"
#include "../common/common.hlsl"

// PackForReblurDiffuse()/PackForReblurSpecular(), used by the RTX-reference
// raygens below to pack IN_DIFF_RADIANCE_HITDIST/IN_SPEC_RADIANCE_HITDIST for
// NRD's REBLUR_DIFFUSE/REBLUR_SPECULAR denoisers (see
// [[project-nrd-integration]]). Self-contained (no NRD_INTERNAL/NRD_METHOD-
// gated code reached), safe to pull into a non-NRD shader -- NRD_INCLUDED
// guards against double-inclusion.
#include "../nrd/reblur_pack_helper.hlsli"


typedef BuiltInTriangleIntersectionAttributes MyAttributes;

float4 get_voxel(float3 pos, float level)
{
	return CreateVoxelScreen().GetVoxels().SampleLevel(linearClampSampler, pos, level);
}

// Cone-trace fallback through the 3D voxel volume, used by MyRaygenShader/
// MyRaygenShaderReflection below when their primary RTX ray misses within
// its short reach. origin/dir are world-space; converted into the voxel
// volume's normalized [0,1]^3 space via voxel_info's min/size. Same
// cone-marching shape as voxel_lighting.hlsl's own trace(), but samples
// VoxelScreen's screen-facing voxel texture (voxels) and falls back to the
// sky cubemap (tex_cube) once the cone has mostly resolved.
float4 trace(VoxelInfo voxel_info, float k, float bias, float3 origin, float3 dir, float angle, out float dist)
{
	dir = normalize(dir);

	float3 voxel_min = voxel_info.GetMin().xyz;
	float3 voxel_size = voxel_info.GetSize().xyz;

	float3 samplePos = (origin - voxel_min) / voxel_size;
	float3 sampleDir = normalize(dir / voxel_size);

	float4 accum = 0;
	float minDiameter = 1.0 / 256;
	float minVoxelDiameterInv = 1.0 / minDiameter;

	float maxDist = 1;
	float d = minDiameter + bias;

	while (d <= maxDist && accum.w < 1 && all(samplePos + sampleDir * d <= 1) && all(samplePos + sampleDir * d >= 0))
	{
		float sampleDiameter = minDiameter + angle * d;
		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
		float3 p = samplePos + sampleDir * d;
		float4 sampleValue = get_voxel(p, sampleLOD);
		float sampleWeight = 1 - accum.w;
		accum += sampleValue * sampleWeight;
		d += sampleDiameter;
	}

	dist = d;

	float4 sky = CreateVoxelScreen().GetTex_cube().SampleLevel(linearSampler, dir, angle * 8);
	float skyWeight = saturate(1 - 8 * accum.w);
	accum += sky * pow(skyWeight, 2);

	return accum;
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, in Camera camera, out float3 origin, out float3 direction)
{
	float2 xy = index + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;



	// Unproject the pixel coordinate into a ray.
	float4 world = mul(camera.GetInvViewProj(), float4(screenPos, 0, 1));

	world.xyz /= world.w;
	origin = camera.GetPosition().xyz;
	direction = normalize(world.xyz - origin);
}





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


float get_occlusion(Camera prev_camera, float prev_z, float2 tc, float3 pos, float vl)
{
	float3 prev_pos = depth_to_wpos(prev_z, tc, prev_camera.GetInvViewProj());

	float l = 10 * length(pos - prev_pos) / vl;

    return 0.00005 + (all(tc > 0)&&all(tc < 1) * saturate(1 - l));
	//return all(tc > 0 && tc < 1) && l < 0.1 ? 1 : 0.005;
}

static const int2 offset[4] =
{
	int2(0, 0),
	int2(0, 1),
	int2(1, 0),
	int2(1, 1)
};
#define FRAMES 16

struct upscale_result
{
	float4 history;
	float frames;
};

upscale_result get_history(VoxelScreen voxel_screen, Camera prev_camera, float3 pos, float2 prev_tc, float2 dims, float l)
{
	Bilinear bilinearFilterAtPrevPos = GetBilinearFilter((prev_tc), dims);
	float2 gatherUv = (float2(bilinearFilterAtPrevPos.origin) + 0.5) / dims;

	float4 viewZprev = voxel_screen.GetPrev_depth().GatherRed(pointBorderSampler, gatherUv).wzxy;
	// Compute disocclusion basing on plane distance

	float4 occlusion;


	occlusion.x = get_occlusion(prev_camera, viewZprev.x, gatherUv + float2(offset[0]) / dims, pos, l);
	occlusion.y = get_occlusion(prev_camera, viewZprev.y, gatherUv + float2(offset[1]) / dims, pos, l);
	occlusion.z = get_occlusion(prev_camera, viewZprev.z, gatherUv + float2(offset[2]) / dims, pos, l);
	occlusion.w = get_occlusion(prev_camera, viewZprev.w, gatherUv + float2(offset[3]) / dims, pos, l);

	// Sample history
	float4 weights = GetBilinearCustomWeights(bilinearFilterAtPrevPos, occlusion);

	float4 s00 = voxel_screen.GetPrev_gi().SampleLevel(pointBorderSampler, gatherUv + float2(offset[0]) / dims, 0);
	float4 s01 = voxel_screen.GetPrev_gi().SampleLevel(pointBorderSampler, gatherUv + float2(offset[1]) / dims, 0);
	float4 s10 = voxel_screen.GetPrev_gi().SampleLevel(pointBorderSampler, gatherUv + float2(offset[2]) / dims, 0);
	float4 s11 = voxel_screen.GetPrev_gi().SampleLevel(pointBorderSampler, gatherUv + float2(offset[3]) / dims, 0);

	float4 accumSpeedPrev = float4(s00.w, s01.w, s10.w, s11.w);

	// ... read s00, s10, s01, s11 using point filtering
	float4 history = ApplyBilinearCustomWeights(s00, s10, s01, s11, weights, true);

	accumSpeedPrev = min(FRAMES * accumSpeedPrev + 1.0, FRAMES);
	//	int newFrames = occlusion ? 0 : clamp(prevFrames + 1, 0, 8);

	float accumSpeed = ApplyBilinearCustomWeights(accumSpeedPrev.x, accumSpeedPrev.y, accumSpeedPrev.z, accumSpeedPrev.w, weights, false);

	upscale_result result;

	result.history = float4(history.xyz, accumSpeed);
	result.frames = accumSpeed;
	return result;
}

//#include "../autogen/DebugInfo.h"

[shader("raygeneration")]
void ShadowRaygenShader()
{

	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;

	float2 tc = float2(itc + 0.5f) / dims;

	const VoxelScreen voxel_screen = CreateVoxelScreen();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const Raytracing raytracing = CreateRaytracing();
	const FrameInfo frame = CreateFrameInfo();

	// DebugInfo info = CreateDebugInfo();

	const RWTexture2D<float4> tex_noise = voxel_output.GetNoise();

	uint2 dd;
	tex_noise.GetDimensions(dd.x, dd.y);
	//info.Log(0,uint4(pass_VoxelOutput.uav_0, dd.x, dd.y, 0));
	float raw_z = voxel_screen.GetGbuffer().GetDepth()[DispatchRaysIndex().xy];
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());
	//float4 gbufer_normals = voxel_screen.GetGbuffer().GetNormals()[DispatchRaysIndex().xy];

	//float3 normal = normalize(gbufer_normals.xyz * 2 - 1);
	//pos += normal / 10000000;

	float shadow = true;
	{

		float hit_rate = 0;
		int samples = 16;// payload2.recursion < 2 ? 3 : 1;
		for (int i = 0; i < samples; i++)
		{
			float3 dir = GetRandomDir(tc, frame.GetSunDir(), 0.02, frame.GetTime() + float(i) / 10);

			ShadowPayload payload_shadow = { false };

			RayDesc ray;
			ray.Origin = pos;
			ray.Direction = dir;
			ray.TMin = 0.1;
			ray.TMax = 10000.0;
			ShadowPass(raytracing.GetScene(), ray, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, payload_shadow);

			if (payload_shadow.hit)
				hit_rate += 1.0f;

		}
		shadow = 1.0 - hit_rate / samples;
	}

	float2 delta = voxel_screen.GetGbuffer().GetMotion().SampleLevel(pointClampSampler, tc, 0).xy;
	float2 prev_tc = tc - delta;

	float l = length(pos - frame.GetCamera().GetPosition());

	upscale_result reprojected = get_history(voxel_screen, frame.GetPrevCamera(), pos, prev_tc, dims, l);

	float speed = 1.0 / (1.0 + reprojected.frames);


	//shadow = lerp(reprojected.history, shadow, speed);





	tex_noise[itc] = float4(shadow.xxx, float(reprojected.frames) / FRAMES);// lerp(tex_noise[itc], shadow, 0.01);// !payload_shadow.hit;
}

// Independent RTX-only reference shadow for ShadowRTX (see voxel.sig's
// PassNode ShadowRTX). Deliberately NOT sharing code with ShadowRaygenShader
// above -- that one stays untouched, still used by RTXShadowReference's
// 16-sample ground truth. This one is genuinely raw: one ray per pixel
// toward a jittered direction within the sun's angular disk (same
// GetRandomDir technique, just 1 sample instead of 16, so the result is
// actually noisy), no temporal history -- a real reference signal, cheap
// enough to eventually feed a denoiser rather than being one itself.
[shader("raygeneration")]
void MyRaygenShaderShadowRTXOnly()
{
	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;
	float2 tc = float2(itc + 0.5f) / dims;

	const VoxelScreen voxel_screen = CreateVoxelScreen();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const Raytracing raytracing = CreateRaytracing();
	const FrameInfo frame = CreateFrameInfo();

	const RWTexture2D<float4> tex_noise = voxel_output.GetNoise();

	float raw_z = voxel_screen.GetGbuffer().GetDepth()[itc];
	if (raw_z == 0)
	{
		tex_noise[itc] = 0;
		return;
	}
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());

	float3 dir = GetRandomDir(tc, frame.GetSunDir(), 0.02, frame.GetTime());

	ShadowPayload payload_shadow = { false };

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dir;
	ray.TMin = 0.1;
	ray.TMax = 10000.0;
	ShadowPass(raytracing.GetScene(), ray, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, payload_shadow);

	float shadow = payload_shadow.hit ? 0.0 : 1.0;
	tex_noise[itc] = float4(shadow.xxx, 1);
}


[shader("raygeneration")]
void ColorPass()
{

	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;

	float2 tc = float2(itc + 0.5f) / dims;

	const VoxelScreen voxel_screen = CreateVoxelScreen();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const Raytracing raytracing = CreateRaytracing();
	const FrameInfo frame = CreateFrameInfo();


	const RWTexture2D<float4> tex_noise = voxel_output.GetNoise();


	float3 pos = depth_to_wpos(0.1, tc, frame.GetCamera().GetInvViewProj());


	RayDesc ray;
	ray.Origin = frame.GetCamera().GetPosition();
	ray.Direction = normalize(pos - frame.GetCamera().GetPosition());
	ray.TMin = 0.1;
	ray.TMax = 10000.0;



	[raypayload]  RayPayload payload_gi;
	payload_gi.color = 0;
	payload_gi.recursion = 0;
	payload_gi.dist = 0;
	payload_gi.cone.angle = 0;
	payload_gi.cone.width = 0;

	TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 1, 0, 1, ray, payload_gi);



	//ShadowPayload payload_shadow = { false };

	//TraceRay(raytracing.GetScene(), RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 0, 0, 0, ray, payload_shadow);

    tex_noise[itc] = float4(payload_gi.color.rgb, 1); // erp(tex_noise[itc], shadow, 0.1);// !payload_shadow.hit;
}


// Indirect-GI reference signal for IndirectRTX (see voxel.sig's PassNode
// IndirectRTX): one ray per pixel, GGX-importance-sampled hemisphere
// direction, fixed reach, no voxel grid involved, no history blend. Denoised
// by NRD REBLUR_DIFFUSE (see [[project-nrd-integration]]).
[shader("raygeneration")]
void MyRaygenShaderIndirectRTXOnly()
{
	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;
	float2 tc = float2(itc + 0.5f) / dims;

	const FrameInfo frame = CreateFrameInfo();
	const Raytracing raytracing = CreateRaytracing();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const VoxelScreen voxel_screen = CreateVoxelScreen();

	const RWTexture2D<float4> tex_noise = voxel_output.GetNoise();

	float raw_z = voxel_screen.GetGbuffer().GetDepth()[itc];
	if (raw_z == 0)
	{
		tex_noise[itc] = 0;
		return;
	}
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());
	float3 normal = normalize(voxel_screen.GetGbuffer().GetNormals()[itc].xyz * 2 - 1);

	float2 seed = voxel_output.GetBlueNoise().Load(int3(itc % 128, 0));
	float3 dir = ImportanceSampleGGX(seed, 1, normal);

	[raypayload]
	RayPayload payload_gi;
	payload_gi.init();

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dir;
	ray.TMin = 0.01;
	// Fixed reach, same convention as this file's other plain-RTX raygens --
	// no voxel grid involved.
	ray.TMax = 10000.0;
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload_gi);

	float viewZ = mul(frame.GetCamera().GetView(), float4(pos, 1)).z;
	tex_noise[itc] = PackForReblurDiffuse(payload_gi.color.rgb, payload_gi.dist, viewZ);
}

// Reflection reference signal for ReflectionRTX (see voxel.sig's PassNode
// ReflectionRTX): pure DXR, one ray per pixel, blue-noise-jittered direction
// (SampleReflectionVector -- this is what makes the output genuinely noisy
// rather than a perfect mirror, which is what a denoiser is meant to clean
// up), fixed TMax, no voxel grid involved. Denoised by NRD REBLUR_SPECULAR
// (see [[project-nrd-integration]]).
[shader("raygeneration")]
void MyRaygenShaderReflectionRTXOnly()
{
	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;
	float2 tc = float2(itc + 0.5f) / dims;

	const FrameInfo frame = CreateFrameInfo();
	const Raytracing raytracing = CreateRaytracing();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const VoxelScreen voxel_screen = CreateVoxelScreen();

	const RWTexture2D<float4> tex_noise = voxel_output.GetNoise();
	const RWTexture2D<float4> tex_dir_pdf = voxel_output.GetDirAndPdf();

	float raw_z = voxel_screen.GetGbuffer().GetDepth()[itc];
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());

	if (raw_z == 0)
	{
		tex_noise[itc] = 0;
		tex_dir_pdf[itc] = 0;
		return;
	}

	float4 gbufer_normals = voxel_screen.GetGbuffer().GetNormals()[itc];
	float4 gbufer_albedo = voxel_screen.GetGbuffer().GetAlbedo()[itc];

	float3 normal = normalize(gbufer_normals.xyz * 2 - 1);
	float roughness = pow(max(MIN_ROUGHNESS, gbufer_normals.w), 2);

	float3 view = -normalize(frame.GetCamera().GetPosition() - pos);

	// Blue-noise-jittered sample around the mirror direction -- one ray,
	// genuinely stochastic (not a perfect reflection), which is the raw
	// signal DLSS-RR's own denoiser is designed to clean up.
	float2 seed = voxel_output.GetBlueNoise().Load(int3(itc % 128, 0));
	float3 dir = SampleReflectionVector(view, normal, roughness, seed);

	[raypayload]
	RayPayload payload;
	payload.color = float4(0, 0, 0, 0);
	payload.recursion = 0;
	payload.dist = 0;
	payload.cone.angle = 0;
	payload.cone.width = 0;

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dir;
	ray.TMin = 0.05;
	// Fixed reach, same convention as this file's other plain-RTX raygens
	// (ShadowRaygenShader/ColorRTXRaygenShader) -- no voxel grid involved.
	ray.TMax = 10000.0;
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload);

	float3 refl_pos = pos + view * clamp(payload.dist, 0, 1000);
	tex_dir_pdf[itc] = float4(refl_pos, 1);

	// Pack for NRD's REBLUR_SPECULAR (see [[project-nrd-integration]]):
	// gbufer_normals.w is the *linear* roughness (same convention
	// NRD_GBufferPack's own NRD_NormalRoughness output uses) -- NOT the
	// squared/alpha `roughness` above, which is this raygen's own GGX-sampling
	// parameter, a different convention for a different purpose.
	float viewZ = mul(frame.GetCamera().GetView(), float4(pos, 1)).z;
	tex_noise[itc] = PackForReblurSpecular(payload.color.rgb, payload.dist, gbufer_normals.w, viewZ);
}


// Indirect-GI voxel-cone-traced signal for VoxelScreen (see voxel.sig's
// PassNode VoxelScreen): RTX primary ray (GGX-importance-sampled hemisphere
// direction, same as MyRaygenShaderIndirectRTXOnly) with a short reach,
// falling back to a cone-trace through the 3D voxel volume on miss. An
// alternative source for NRD REBLUR_DIFFUSE, selected against IndirectRTX's
// raw RTX reference via g_indirect_source (see [[project-nrd-integration]]).
[shader("raygeneration")]
void MyRaygenShader()
{
	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;
	float2 tc = float2(itc + 0.5f) / dims;

	const FrameInfo frame = CreateFrameInfo();
	const Raytracing raytracing = CreateRaytracing();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const VoxelScreen voxel_screen = CreateVoxelScreen();
	const VoxelInfo voxel_info = CreateVoxelInfo();

	const RWTexture2D<float4> tex_noise_raw = voxel_output.GetNoiseRaw();

	float raw_z = voxel_screen.GetGbuffer().GetDepth()[itc];
	if (raw_z == 0)
	{
		tex_noise_raw[itc] = 0;
		return;
	}
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());
	float3 normal = normalize(voxel_screen.GetGbuffer().GetNormals()[itc].xyz * 2 - 1);

	float2 seed = voxel_output.GetBlueNoise().Load(int3(itc % 128, 0));
	float3 dirVoxel = ImportanceSampleGGX(seed, 1, normal);

	[raypayload]
	RayPayload payload_gi;
	payload_gi.init();

	float3 oneVoxelSize = voxel_info.GetSize().xyz / (voxel_info.GetVoxel_tiles_count().xyz * voxel_info.GetVoxels_per_tile().xyz);

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dirVoxel;
	ray.TMin = 0.01;
	ray.TMax = length(oneVoxelSize) * 8;
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload_gi);

	[branch]
	if (payload_gi.dist > 100000 - 5)
	{
		payload_gi.color = trace(voxel_info, 0, 0.0, pos + dirVoxel * ray.TMax, dirVoxel, 0.2, payload_gi.dist);
	}

	float viewZ = mul(frame.GetCamera().GetView(), float4(pos, 1)).z;
	tex_noise_raw[itc] = PackForReblurDiffuse(payload_gi.color.rgb, payload_gi.dist, viewZ);
}

// Reflection voxel-cone-traced signal for ScreenReflection (see voxel.sig's
// PassNode ScreenReflection): RTX primary ray (SampleReflectionVector, same
// as MyRaygenShaderReflectionRTXOnly) with a roughness-dependent short reach,
// falling back to a cone-trace through the 3D voxel volume on miss. An
// alternative source for NRD REBLUR_SPECULAR, selected against
// ReflectionRTX's raw RTX reference via g_reflection_source (see
// [[project-nrd-integration]]).
[shader("raygeneration")]
void MyRaygenShaderReflection()
{
	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;
	float2 tc = float2(itc + 0.5f) / dims;

	const FrameInfo frame = CreateFrameInfo();
	const Raytracing raytracing = CreateRaytracing();
	const VoxelOutput voxel_output = CreateVoxelOutput();
	const VoxelScreen voxel_screen = CreateVoxelScreen();
	const VoxelInfo voxel_info = CreateVoxelInfo();

	const RWTexture2D<float4> tex_noise_raw = voxel_output.GetNoiseRaw();

	float raw_z = voxel_screen.GetGbuffer().GetDepth()[itc];
	if (raw_z == 0)
	{
		tex_noise_raw[itc] = 0;
		return;
	}
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());

	float4 gbufer_normals = voxel_screen.GetGbuffer().GetNormals()[itc];
	float3 normal = normalize(gbufer_normals.xyz * 2 - 1);
	float roughness = pow(max(MIN_ROUGHNESS, gbufer_normals.w), 2);

	float3 view = -normalize(frame.GetCamera().GetPosition() - pos);

	float2 seed = voxel_output.GetBlueNoise().Load(int3(itc % 128, 0));
	float3 dirVoxel = SampleReflectionVector(view, normal, roughness, seed);

	[raypayload]
	RayPayload payload_gi;
	payload_gi.color = float4(dirVoxel, 0);
	payload_gi.recursion = 0;
	payload_gi.dist = 0;
	payload_gi.cone.angle = 0;
	payload_gi.cone.width = 0;

	float3 oneVoxelSize = voxel_info.GetSize().xyz / (voxel_info.GetVoxel_tiles_count().xyz * voxel_info.GetVoxels_per_tile().xyz);

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dirVoxel;
	ray.TMin = 0.05;
	ray.TMax = 0.5 * length(oneVoxelSize) / (tan(roughness + 0.001) + 0.001);
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload_gi);

	[branch]
	if (payload_gi.dist > 100000 - 5)
	{
		payload_gi.color = trace(voxel_info, 0, 1 * seed.x * length(oneVoxelSize), pos + oneVoxelSize * normal + 1 * dirVoxel * ray.TMax, dirVoxel, 1 * roughness / 2, payload_gi.dist);
	}

	float viewZ = mul(frame.GetCamera().GetView(), float4(pos, 1)).z;
	tex_noise_raw[itc] = PackForReblurSpecular(payload_gi.color.rgb, payload_gi.dist, gbufer_normals.w, viewZ);
}


[shader("miss")]
void MyMissShader([raypayload]inout
RayPayload payload)
{
	// Missed all geometry -> sample the sky cubemap in the ray direction so
	// reflections/refractions and primary rays show the real environment.
	float3 sky = CreateFrameInfo().GetSky().SampleLevel(linearSampler, normalize(WorldRayDirection()), 0);
	payload.color = float4(sky, 1.0);
	payload.dist = 100000;
}



[shader("closesthit")]
void ShadowClosestHitShader([raypayload] inout
ShadowPayload payload, in MyAttributes attr)
{
	payload.hit = true;
}


[shader("miss")]
void ShadowMissShader([raypayload] inout
ShadowPayload payload)
{
	payload.hit = false;
}


[shader("miss")]
void ColorShadowMissShader([raypayload] inout ColorShadowPayload payload)
{
	// Nothing occluded the ray -> the light fully reaches; leave transmittance as-is.
	payload.dist = -1.0; // signal "no hit" so the caller stops advancing.
}

#endif // RAYTRACING_HLSL