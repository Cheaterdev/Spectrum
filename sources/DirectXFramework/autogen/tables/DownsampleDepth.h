#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DownsampleDepth
	{
		Render::HLSL::Texture2D<float> srcTex;
		Render::HLSL::RWTexture2D<float> targetTex;
		Render::HLSL::Texture2D<float>& GetSrcTex() { return srcTex; }
		Render::HLSL::RWTexture2D<float>& GetTargetTex() { return targetTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(srcTex);
			compiler.compile(targetTex);
		}
	};
	#pragma pack(pop)
}
