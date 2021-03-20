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
#include "autogen/RaytracingRays.h"

#include "autogen/tables/RayPayload.h"
#include "autogen/tables/ShadowPayload.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelInfo.h"

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
	accum.xyz += sky * pow(sampleWeight, 1);

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


[shader("raygeneration")]
void MyRaygenShader()
{
 //  float3 rayDir = float3(0,1,0);
 //   float3 origin = float3(0, 1, 0);;
    
	float2 tc = float2(DispatchRaysIndex().xy)/DispatchRaysDimensions().xy;

    const FrameInfo frame = CreateFrameInfo();

    const Raytracing raytracing = CreateRaytracing();
     const RaytracingRays rays = CreateRaytracingRays();
	 const RWTexture2D<float4> output = rays.GetOutput();
	 
  
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
   
	float raw_z = rays.GetGbuffer().GetDepth()[DispatchRaysIndex().xy];
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());


	float3 normal = normalize(rays.GetGbuffer().GetNormals()[DispatchRaysIndex().xy].xyz * 2 - 1);

	//float3 lightDir = frame.GetSunDir();

//float3 view = -normalize(frame.GetCamera().GetPosition() - pos);

//float3 rayDir = reflect(view, normal);


float time = frame.GetTime().y;// +i;
float sini = sin(time * 220 + float(tc.x));
float cosi = cos(time * 220 + float(tc.y));
float rand = rnd(float2(sini, cosi));


float rcos = cos(6.14 * rand);
float rsin = sin(6.14 * rand);
float rand2 = rnd(float2(cosi, sini));

float tt =  2 * pow(rand2, 1.0 / 3.0);

float3 right = rsin *  normalize(cross(normal, float3(0, 1, 0.1)));
float3 tangent = rcos * normalize(cross(right, normal));

float3 dir = normalize(normal + tt * (right + tangent));

float3 dirVoxel = normalize(normal + rand2*(right + tangent));


RayPayload payload_shadow;
payload_shadow.color = float4(dirVoxel,0);
payload_shadow.recursion = 0;
payload_shadow.dist = 0;
payload_shadow.cone.angle = 0;
payload_shadow.cone.width = 0;


//GenerateCameraRay(DispatchRaysIndex().xy, frame.GetCamera(), pos, dir);

{
	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = dir;
	ray.TMin = 0.1;
	ray.TMax = 3.0;
	TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload_shadow);

	if (payload_shadow.dist > 100)
	{
			payload_shadow.color = trace(0, 0.0, pos + dirVoxel * 3, dirVoxel, 0.4);
	}
	output[DispatchRaysIndex().xy] = lerp(output[DispatchRaysIndex().xy], payload_shadow.color, 0.01);
}


	/*{

	ShadowPayload payload_shadow = { false };

	RayDesc ray;
	ray.Origin = pos;
	ray.Direction = float3(0,1,0);
	ray.TMin = 0.003;
	ray.TMax = 1000.0;
	TraceRay(raytracing.GetScene(), RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 1, 0, 1, ray, payload_shadow);

	output[DispatchRaysIndex().xy] = payload_shadow.hit;
	}*/
			//trace(0, pos+dir, dir, 0.2); //
}



/*
[shader("raygeneration")]
void MyRaygenShader()
{
	  float3 rayDir = float3(0,1,0);
	   float3 origin = float3(0, 1, 0);;

	float2 tc = float2(DispatchRaysIndex().xy) / DispatchRaysDimensions().xy;

	const FrameInfo frame = CreateFrameInfo();

	const Raytracing raytracing = CreateRaytracing();
	const RaytracingRays rays = CreateRaytracingRays();
	const RWTexture2D<float4> output = rays.GetOutput();


	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, frame.GetCamera(), origin, rayDir);

	


	RayPayload payload_shadow;
	payload_shadow.color = 0;
	payload_shadow.recursion = 0;
	payload_shadow.dist = 0;
	payload_shadow.cone.angle = 0;
	payload_shadow.cone.width = 0;

	RayDesc ray;
	ray.Origin = origin;
	ray.Direction = rayDir;
	ray.TMin = 0.03;
	ray.TMax = 1000.0;
	TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload_shadow);

	output[DispatchRaysIndex().xy] = lerp(output[DispatchRaysIndex().xy], payload_shadow.color, 0.1);

}
*/

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
	
	// CreateFrameInfo().GetSky().SampleLevel(linearSampler, normalize(WorldRayDirection()), 3);
	payload.dist = 10000;
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