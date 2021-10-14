#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelDebug
	{
		struct SRV
		{
			Render::HLSL::Texture3D<float4> volume;
			GBuffer::SRV gbuffer;
		} &srv;
		Render::HLSL::Texture3D<float4>& GetVolume() { return srv.volume; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelDebug(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
