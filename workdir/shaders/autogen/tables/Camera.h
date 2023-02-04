#pragma once
#include "Frustum.h"
struct Camera
{
	float4x4 view; // float4x4
	float4x4 proj; // float4x4
	float4x4 viewProj; // float4x4
	float4x4 invView; // float4x4
	float4x4 invProj; // float4x4
	float4x4 invViewProj; // float4x4
	float4x4 reprojectionProj; // float4x4
	float4 position; // float4
	float4 direction; // float4
	float4 jitter; // float4
	Frustum frustum; // Frustum
	float4x4 GetView() { return view; }
	float4x4 GetProj() { return proj; }
	float4x4 GetViewProj() { return viewProj; }
	float4x4 GetInvView() { return invView; }
	float4x4 GetInvProj() { return invProj; }
	float4x4 GetInvViewProj() { return invViewProj; }
	float4x4 GetReprojectionProj() { return reprojectionProj; }
	float4 GetPosition() { return position; }
	float4 GetDirection() { return direction; }
	float4 GetJitter() { return jitter; }
	Frustum GetFrustum() { return frustum; }
};
