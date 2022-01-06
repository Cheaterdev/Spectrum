#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsample
	{
		Render::HLSL::Texture2D<float4> normals;
		Render::HLSL::Texture2D<float> depth;
		Render::HLSL::Texture2D<float4>& GetNormals() { return normals; }
		Render::HLSL::Texture2D<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(normals);
			compiler.compile(depth);
		}
	};
	#pragma pack(pop)
}
