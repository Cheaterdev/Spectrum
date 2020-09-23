#pragma once
#include "Glyph.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingGlyphs
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle data;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetData() { return srv.data; }
		FontRenderingGlyphs(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
