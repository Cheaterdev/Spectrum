#pragma once
#include "Frustum.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct Camera
	{
		struct CB
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
			Frustum::CB frustum;
		} &cb;
		float4x4& GetView() { return cb.view; }
		float4x4& GetProj() { return cb.proj; }
		float4x4& GetViewProj() { return cb.viewProj; }
		float4x4& GetInvView() { return cb.invView; }
		float4x4& GetInvProj() { return cb.invProj; }
		float4x4& GetInvViewProj() { return cb.invViewProj; }
		float4& GetPosition() { return cb.position; }
		float4& GetDirection() { return cb.direction; }
		float4& GetJitter() { return cb.jitter; }
		Frustum MapFrustum() { return Frustum(cb.frustum); }
		Camera(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
