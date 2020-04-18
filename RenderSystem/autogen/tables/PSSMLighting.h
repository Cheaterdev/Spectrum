#pragma once
#include "GBuffer.h"
namespace Table 
{
	struct PSSMLighting
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle light_mask;
			Render::Handle brdf;
			GBuffer::SRV gbuffer;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetLight_mask() { return srv.light_mask; }
		Render::Handle& GetBrdf() { return srv.brdf; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		PSSMLighting(SRV&srv) :srv(srv){}
	};
}
