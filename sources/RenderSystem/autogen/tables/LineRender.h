#pragma once
#include "VSLine.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct LineRender
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<VSLine> vb;
		} &srv;
		DX12::HLSL::StructuredBuffer<VSLine>& GetVb() { return srv.vb; }
		LineRender(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
