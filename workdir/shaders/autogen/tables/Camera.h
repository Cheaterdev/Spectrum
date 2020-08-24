#pragma once
#include "Frustum.h"
struct Camera_cb
{
	float4x4 view;
	float4x4 proj;
	float4x4 viewProj;
	float4x4 invView;
	float4x4 invProj;
	float4x4 invViewProj;
	float4 position;
	float4 direction;
	float4 jitter;
	Frustum_cb frustum;
};
struct Camera
{
	Camera_cb cb;
	float4x4 GetView() { return cb.view; }
	float4x4 GetProj() { return cb.proj; }
	float4x4 GetViewProj() { return cb.viewProj; }
	float4x4 GetInvView() { return cb.invView; }
	float4x4 GetInvProj() { return cb.invProj; }
	float4x4 GetInvViewProj() { return cb.invViewProj; }
	float4 GetPosition() { return cb.position; }
	float4 GetDirection() { return cb.direction; }
	float4 GetJitter() { return cb.jitter; }
	Frustum GetFrustum() { return CreateFrustum(cb.frustum); }

};
 const Camera CreateCamera(Camera_cb cb)
{
	const Camera result = {cb
	};
	return result;
}
