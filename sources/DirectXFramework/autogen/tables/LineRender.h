#pragma once
#include "VSLine.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct LineRender
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<VSLine> vb;
		} &srv;
		Render::HLSL::StructuredBuffer<VSLine>& GetVb() { return srv.vb; }
		LineRender(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
