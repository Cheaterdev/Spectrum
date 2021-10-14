#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMLighting
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float> light_mask;
			GBuffer::SRV gbuffer;
		} &srv;
		Render::HLSL::Texture2D<float>& GetLight_mask() { return srv.light_mask; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		PSSMLighting(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
