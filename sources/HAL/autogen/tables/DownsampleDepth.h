#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DownsampleDepth
	{
		HLSL::Texture2D<float> srcTex;
		HLSL::RWTexture2D<float> targetTex;
		HLSL::Texture2D<float>& GetSrcTex() { return srcTex; }
		HLSL::RWTexture2D<float>& GetTargetTex() { return targetTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(srcTex);
			compiler.compile(targetTex);
		}
		struct Compiled
		{
			uint srcTex; // Texture2D<float>
			uint targetTex; // RWTexture2D<float>
		};
	};
	#pragma pack(pop)
}
