#pragma once
namespace Table 
{
	struct FontRenderingConstants
	{
		struct CB
		{
			float4x4 TransformMatrix;
			float4 ClipRect;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4x4& GetTransformMatrix() { return cb.TransformMatrix; }
		float4& GetClipRect() { return cb.ClipRect; }
		FontRenderingConstants(CB&cb) :cb(cb){}
	};
}
