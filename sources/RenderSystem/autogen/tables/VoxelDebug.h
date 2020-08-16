#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelDebug
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle volume;
			GBuffer::SRV gbuffer;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetVolume() { return srv.volume; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelDebug(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
