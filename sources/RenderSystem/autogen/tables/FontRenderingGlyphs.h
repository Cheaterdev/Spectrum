#pragma once
#include "Glyph.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingGlyphs
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<Glyph> data;
		} &srv;
		DX12::HLSL::StructuredBuffer<Glyph>& GetData() { return srv.data; }
		FontRenderingGlyphs(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
