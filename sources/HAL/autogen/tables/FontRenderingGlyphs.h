#pragma once
#include "Glyph.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingGlyphs
	{
		HLSL::StructuredBuffer<Glyph> data;
		HLSL::StructuredBuffer<Glyph>& GetData() { return data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
		}
		struct Compiled
		{
			uint data; // StructuredBuffer<Glyph>
		};
	};
	#pragma pack(pop)
}
