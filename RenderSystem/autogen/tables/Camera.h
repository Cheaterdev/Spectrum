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
			float3 position;
			float unused;
			float3 direction;
			float unused2;
			float2 jitter;
			float2 unused3;
			Frustum::CB frustum;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4x4& GetView() { return cb.view; }
		float4x4& GetProj() { return cb.proj; }
		float4x4& GetViewProj() { return cb.viewProj; }
		float4x4& GetInvView() { return cb.invView; }
		float4x4& GetInvProj() { return cb.invProj; }
		float4x4& GetInvViewProj() { return cb.invViewProj; }
		float3& GetPosition() { return cb.position; }
		float& GetUnused() { return cb.unused; }
		float3& GetDirection() { return cb.direction; }
		float& GetUnused2() { return cb.unused2; }
		float2& GetJitter() { return cb.jitter; }
		float2& GetUnused3() { return cb.unused3; }
		Frustum MapFrustum() { return Frustum(cb.frustum); }
		Camera(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
