#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelDebug
	{
		struct SRV
		{
			DX12::HLSL::Texture3D<float4> volume;
			GBuffer::SRV gbuffer;
		} &srv;
		DX12::HLSL::Texture3D<float4>& GetVolume() { return srv.volume; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelDebug(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
