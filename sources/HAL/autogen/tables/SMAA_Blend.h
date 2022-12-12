#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Blend
	{
		HLSL::Texture2D<float4> blendTex;
		HLSL::Texture2D<float4>& GetBlendTex() { return blendTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(blendTex);
		}
		struct Compiled
		{
			uint blendTex; // Texture2D<float4>
		};
	};
#pragma pack(pop)
				}
