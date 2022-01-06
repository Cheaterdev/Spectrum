#pragma once
#include "Glyph.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingGlyphs
	{
		Render::HLSL::StructuredBuffer<Glyph> data;
		Render::HLSL::StructuredBuffer<Glyph>& GetData() { return data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
		}
	};
	#pragma pack(pop)
}
