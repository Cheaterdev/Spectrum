#pragma once
#include "Glyph.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingGlyphs
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<Glyph> data;
		} &srv;
		Render::HLSL::StructuredBuffer<Glyph>& GetData() { return srv.data; }
		FontRenderingGlyphs(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
