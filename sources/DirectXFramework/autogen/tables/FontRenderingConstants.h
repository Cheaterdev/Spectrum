#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingConstants
	{
		struct CB
		{
			float4x4 TransformMatrix;
			float4 ClipRect;
		} &cb;
		float4x4& GetTransformMatrix() { return cb.TransformMatrix; }
		float4& GetClipRect() { return cb.ClipRect; }
		FontRenderingConstants(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
