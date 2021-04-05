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


#include "autogen/FrameInfo.h"
#include "autogen/Raytracing.h"
//#include "autogen/RaytracingRays.h"

#include "autogen/tables/RayPayload.h"
#include "autogen/tables/ShadowPayload.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelInfo.h"


#include "autogen/VoxelOutput.h"


typedef BuiltInTriangleIntersectionAttributes MyAttributes;


//static const Texture3D<float4> voxels = GetVoxelScreen().GetVoxels();
//static const TextureCube<float4> tex_cube = GetVoxelScreen().GetTex_cube();
//static const Texture2D<float2> speed_tex = GetVoxelScreen().GetGbuffer().GetMotion();



float4 get_voxel(float3 pos, float level)
{
	float4 color = CreateVoxelScreen().GetVoxels().SampleLevel(linearSampler, pos, level);
	return color;
}



float4 trace(float4 start_color, float start_dist,  float3 origin, float3 dir, float angle)
{
	float3 voxel_min = CreateVoxelInfo().GetMin().xyz;
	float3 voxel_size = CreateVoxelInfo().GetSize().xyz;

	//float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14);

//	float angle_coeff = saturate(max_angle / (angle + 0.01));
	//angle = min(angle, max_angle);
	float3 startOrigin = saturate(((origin + start_dist * dir - (voxel_min)) / voxel_size));
	origin = saturate(((origin - (voxel_min)) / voxel_size));
	
	float3 samplePos = 0;
	float4 accum = start_color;
	// the starting sample diameter
	float minDiameter = 1.0 / 512;// *(1 + 4 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;

	float maxDist = 1;
	float dist = length(startOrigin - origin);

	float max_accum = 0.9;
	while (dist <= maxDist && accum.w < max_accum && all(samplePos <= 1) && all(samplePos >= 0))
	{
		float sampleDiameter = minDiameter + angle * dist;

		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
		samplePos = origin + dir * dist;
		float4 sampleValue = get_voxel(samplePos, sampleLOD);//* float4(1,1,1,1 + sampleLOD/4);
		//sampleValue.w *= 333;


		float sampleWeight = saturate(1 - accum.w);
		accum += sampleValue * sampleWeight;
		dist += sampleDiameter;

		//if (accum.w >= 0.9)
		//	accum /= accum.w;

	}
	//accum.xyz *= pow(dist,0.7);
	//accum.xyz *= angle_coeff;
	//accum *= 1.0 / 0.9;

	float3 sky = CreateFrameInfo().GetSky().SampleLevel(linearSampler, normalize(dir), angle);
	float sampleWeight = saturate(max_accum - accum.w ) / max_accum;
	accum.xyz += sky *pow(sampleWeight, 1);

	
	
//	accum.xyz = sky;

	return accum;// / saturate(accum.w);
}



// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index,in Camera camera, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;



    // Unproject the pixel coordinate into a ray.
   float4 world = mul(camera.GetInvViewProj(),float4(screenPos, 0, 1) );

    world.xyz /= world.w;
    origin = camera.GetPosition().xyz;
    direction = normalize(world.xyz - origin);
}


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}
float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
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

	return  0.00005 + (all(tc > 0 && tc < 1) * saturate(1 - l));
	//return all(tc > 0 && tc < 1) && l < 0.1 ? 1 : 0.005;
}

static const int2 offset[4] =
{
	int2(0, 0),
	int2(0, 1),
	int2(1, 0),
	int2(1, 1)
};
#define FRAMES 8

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




[shader("raygeneration")]
void MyRaygenShader()
{
 //  float3 rayDir = float3(0,1,0);
 //   float3 origin = float3(0, 1, 0);;

	uint2 itc = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;
	
	float2 tc = float2(itc + 0.5f)/ dims;

    const FrameInfo frame = CreateFrameInfo();

   const Raytracing raytracing = CreateRaytracing();

	 const VoxelOutput voxel_output = CreateVoxelOutput();
	 const VoxelScreen voxel_screen = CreateVoxelScreen();


Texture2D<float2> speed_tex = voxel_screen.GetGbuffer().GetMotion();

	
	// const RWTexture2D<float4> output = rays.GetOutput();
	 const RWTexture2D<float4> tex_noise = voxel_output.GetNoise();
	 const RWTexture2D<float> tex_frames = voxel_output.GetFrames();
  
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
   
	float raw_z = voxel_screen.GetGbuffer().GetDepth()[DispatchRaysIndex().xy];
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());


	float3 normal = normalize(voxel_screen.GetGbuffer().GetNormals()[DispatchRaysIndex().xy].xyz * 2 - 1);

	//float3 lightDir = frame.GetSunDir();

//float3 view = -normalize(frame.GetCamera().GetPosition() - pos);

//float3 rayDir = reflect(view, normal);


float time = frac(frame.GetTime().y)*5;// +i;
float sini = sin(time * 220 + float(tc.x));
float cosi = cos(time * 220 + float(tc.y));
float rand = rnd(float2(sini, cosi));


float rcos = cos(6.14 * rand);
float rsin = sin(6.14 * rand);
float rand2 = rnd(float2(cosi, sini));

float tt =  4 * pow(rand2, 1.0 / 3.0);

//float tt = rand2;

	
float3 right = rsin *  normalize(cross(normal, float3(0, 1, 0.1)));
float3 tangent = rcos * normalize(cross(right, normal));

float3 dir = normalize(normal + tt * (right + tangent));

float3 dirVoxel = normalize(normal + rand2 *(right + tangent));


RayPayload payload_gi;
payload_gi.color = float4(dirVoxel,0);
payload_gi.recursion = 0;
payload_gi.dist = 0;
payload_gi.cone.angle = 0;
payload_gi.cone.width = 0;


//dir = normalize(pos - frame.GetCamera().GetPosition());
//pos = frame.GetCamera().GetPosition();

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dir;
	ray.TMin = 0.1;
	ray.TMax = 10.0;
	TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload_gi);

	if (payload_gi.dist > 100000-5)
	{
		payload_gi.color = trace(0, 0.0, pos + dirVoxel * 1, dirVoxel, 0.4);
	}
		
	//tex_noise[DispatchRaysIndex().xy] = 1;// lerp(tex_noise[DispatchRaysIndex().xy], payload_shadow.color, 0.01);

/*	{

		ShadowPayload payload_shadow;
		payload_shadow.hit = false;
	
		
		RayDesc ray;
		ray.Origin = pos;
		ray.Direction = frame.GetSunDir();
		ray.TMin = 0.1;
		ray.TMax = 30000.0;
		TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 1, 0, 1, ray, payload_shadow);

		payload_gi.color = payload_shadow.hit;
	}
	*/
	

	float2 delta =  voxel_screen.GetGbuffer().GetMotion().SampleLevel(pointClampSampler, tc, 0).xy;
	float2 prev_tc = tc - delta;

	float l = length(pos - frame.GetCamera().GetPosition());

	upscale_result reprojected = get_history(voxel_screen, frame.GetPrevCamera(), pos, prev_tc, dims, l);

	float speed = 1.0 / (1.0 + reprojected.frames);

	float4 gi = payload_gi.color;// max(0, getGI(itc, pos, pos + scaler * normal / m, normal, v, r, gbuffer.GetNormals()[tc].w, albedo.w));

	gi =  lerp(reprojected.history, gi, speed);

	tex_noise[itc] = float4(gi.xyz, raw_z);// accumSpeedPrev / 8;// (accumSpeed / 8) == 1;
	tex_frames[itc] = float(reprojected.frames) / FRAMES;
	
		

}



[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
	
	// CreateFrameInfo().GetSky().SampleLevel(linearSampler, normalize(WorldRayDirection()), 3);
	payload.dist = 100000;
}



[shader("closesthit")]
void ShadowClosestHitShader(inout ShadowPayload payload, in MyAttributes attr)
{
	payload.hit = true;
}


[shader("miss")]
void ShadowMissShader(inout ShadowPayload payload)
{
	payload.hit = false;
}

#endif // RAYTRACING_HLSL