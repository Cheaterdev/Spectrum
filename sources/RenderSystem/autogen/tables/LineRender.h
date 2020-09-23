#pragma once
#include "VSLine.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct LineRender
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle vb;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetVb() { return srv.vb; }
		LineRender(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
