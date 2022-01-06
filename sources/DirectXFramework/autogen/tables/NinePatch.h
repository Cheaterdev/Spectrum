#pragma once
#include "vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct NinePatch
	{
		Render::HLSL::StructuredBuffer<vertex_input> vb;
		std::vector<Render::HLSL::Texture2D<float4>> textures;
		Render::HLSL::StructuredBuffer<vertex_input>& GetVb() { return vb; }
		std::vector<Render::HLSL::Texture2D<float4>>& GetTextures() { return textures; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vb);
			compiler.compile(textures);
		}
	};
	#pragma pack(pop)
}
