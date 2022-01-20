#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBuffer
	{
		HLSL::Texture2D<float4> albedo;
		HLSL::Texture2D<float4> normals;
		HLSL::Texture2D<float4> specular;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float2> motion;
		HLSL::Texture2D<float4>& GetAlbedo() { return albedo; }
		HLSL::Texture2D<float4>& GetNormals() { return normals; }
		HLSL::Texture2D<float4>& GetSpecular() { return specular; }
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		HLSL::Texture2D<float2>& GetMotion() { return motion; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(albedo);
			compiler.compile(normals);
			compiler.compile(specular);
			compiler.compile(depth);
			compiler.compile(motion);
		}
	};
	#pragma pack(pop)
}
