#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsample
	{
		HLSL::Texture2D<float4> normals;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4>& GetNormals() { return normals; }
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(normals);
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint normals; // Texture2D<float4>
			uint depth; // Texture2D<float>
		};
	};
	#pragma pack(pop)
}
