#pragma once
#include "TilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct TilingPostprocess
	{
		struct SRV
		{
			TilingParams::SRV tiling;
		} &srv;
		TilingParams MapTiling() { return TilingParams(srv.tiling); }
		TilingPostprocess(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
