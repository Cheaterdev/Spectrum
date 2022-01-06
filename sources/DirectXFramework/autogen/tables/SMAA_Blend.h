#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Blend
	{
		Render::HLSL::Texture2D<float4> blendTex;
		Render::HLSL::Texture2D<float4>& GetBlendTex() { return blendTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(blendTex);
		}
	};
	#pragma pack(pop)
}
