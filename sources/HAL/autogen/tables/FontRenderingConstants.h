#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingConstants
	{
		float4x4 TransformMatrix;
		float4 ClipRect;
		float4x4& GetTransformMatrix() { return TransformMatrix; }
		float4& GetClipRect() { return ClipRect; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(TransformMatrix);
			compiler.compile(ClipRect);
		}
	};
	#pragma pack(pop)
}