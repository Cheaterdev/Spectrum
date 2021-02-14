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

typedef BuiltInTriangleIntersectionAttributes MyAttributes;


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
  //  GenerateCameraRay(DispatchRaysIndex().xy, frame.GetCamera(), origin, rayDir);
	
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

float tt = 4;// 0.1 * pow(rand2, 1.0 / 3.0);

float3 right = rsin * tt * normalize(cross(normal, float3(0, 1, 0.1)));
float3 tangent = rcos * tt * normalize(cross(right, normal));

float3 dir = normalize(normal + right + tangent);

	

RayPayload payload_shadow;
payload_shadow.color = 0;
payload_shadow.recursion = 0;
payload_shadow.dist = 0;
payload_shadow.cone.angle = 0;
payload_shadow.cone.width = 0;

			RayDesc ray;
			ray.Origin = pos;
			ray.Direction = dir;
			ray.TMin = 0.25;
			ray.TMax = 1000.0;
			TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0,  0, 0, 0, ray, payload_shadow);
			
			output[DispatchRaysIndex().xy] = lerp(output[DispatchRaysIndex().xy],payload_shadow.color,0.05);

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
	payload.color = CreateFrameInfo().GetSky().SampleLevel(linearSampler, normalize(WorldRayDirection()), 3);
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