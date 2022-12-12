#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CopyTexture
	{
		HLSL::Texture2D<float4> srcTex;
		HLSL::Texture2D<float4>& GetSrcTex() { return srcTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(srcTex);
		}
		struct Compiled
		{
			uint srcTex; // Texture2D<float4>
		};
	};
#pragma pack(pop)
				}
