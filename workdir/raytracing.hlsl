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
struct vertex_input
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 tc : TEXCOORD;
	float4 tangent : TANGENT;
	//float3 binormal : BINORMAL;
};


struct camera_info
{
	matrix view;
	matrix proj;
	matrix view_proj;
	matrix inv_view;
	matrix inv_proj;
	matrix inv_view_proj;
	float3 position;
	float unused;
	float3 direction;
	float unused2;
	float2 jitter; float2 unused3;
};

struct InstanceData
{
	uint vertexBuffer;
	uint indexBuffer;
	uint materialOffset;
};

cbuffer cbCamera : register(b0)
{
	camera_info camera;
};


RaytracingAccelerationStructure Scene : register(t0, space0);


Texture2D<float4> gbuffer[3] : register(t0, space1);
Texture2D<float> depth_buffer : register(t3, space1);

StructuredBuffer<InstanceData> instances: register(t1);
StructuredBuffer<uint> index_buffers[]: register(t0, space3);
StructuredBuffer<vertex_input> vertex_buffers[]: register(t0, space2);


RWTexture2D<float4> RenderTarget : register(u0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
	uint recursion;
	float dist;
};

struct ShadowPayload
{
	bool hit;
};


// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;



    // Unproject the pixel coordinate into a ray.
   float4 world = mul(camera.inv_view_proj,float4(screenPos, 0, 1) );

    world.xyz /= world.w;
    origin = camera.position.xyz;
    direction = normalize(world.xyz - origin);
}


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}


[shader("raygeneration")]
void MyRaygenShader()
{
   float3 rayDir;
    float3 origin;
    
	float2 tc = float2(DispatchRaysIndex().xy)/DispatchRaysDimensions().xy;
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);
	/*
	float raw_z = depth_buffer[DispatchRaysIndex().xy];
	float3 pos = depth_to_wpos(raw_z, tc, camera.inv_view_proj);


	float3 normal = normalize(gbuffer[1][DispatchRaysIndex().xy].xyz * 2 - 1);


float3 view = -normalize(camera.position - pos);

rayDir =  normalize(float3(2,1,1.3));//reflect(view, normal);

origin = pos;*/
        // Trace the ray.
        // Set the ray's extents.
        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = rayDir;
        // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
        // TMin should be kept small to prevent missing geometry at close contact areas.
        ray.TMin = 0.1;
        ray.TMax = 1000.0;
   
		RayPayload payload = { float4(0, 1, 0, 1),0,0 };
        TraceRay(Scene,  RAY_FLAG_NONE    , ~0, 0, 0, 0, ray, payload);
        // Write the raytraced color to the output texture.
		RenderTarget[DispatchRaysIndex().xy] = float4(pow(payload.color.xyz,1.0/1.8),1);// float4(pow(gbuffer[0][DispatchRaysIndex().xy].xyz, 0.5) * (0.1 + payload.color.x * max(0, dot(ray.Direction, normal))), 1);
 
		/*ShadowPayload payload = {false };
		TraceRay(Scene, RAY_FLAG_NONE, ~0, 1, 0, 1, ray, payload);
		// Write the raytraced color to the output texture.
		RenderTarget[DispatchRaysIndex().xy] = float4(0,payload.hit,0,1);// float4(payload.color.xyz, 1);// float4(pow(gbuffer[0][DispatchRaysIndex().xy].xyz, 0.5) * (0.1 + payload.color.x * max(0, dot(ray.Direction, normal))), 1);
		*/
		

}


[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
	payload.color = float4(0.1, 0.1, 0.1, 1);
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