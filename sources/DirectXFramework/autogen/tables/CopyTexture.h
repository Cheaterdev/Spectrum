#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CopyTexture
	{
		Render::HLSL::Texture2D<float4> srcTex;
		Render::HLSL::Texture2D<float4>& GetSrcTex() { return srcTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(srcTex);
		}
	};
	#pragma pack(pop)
}
