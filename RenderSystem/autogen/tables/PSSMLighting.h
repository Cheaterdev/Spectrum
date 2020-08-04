#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMLighting
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle light_mask;
			GBuffer::SRV gbuffer;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetLight_mask() { return srv.light_mask; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		PSSMLighting(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
