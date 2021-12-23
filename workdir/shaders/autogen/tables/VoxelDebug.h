#pragma once
#include "GBuffer.h"
struct VoxelDebug_srv
{
	uint volume; // Texture3D<float4>
	GBuffer_srv gbuffer;
};
struct VoxelDebug
{
	VoxelDebug_srv srv;
	Texture3D<float4> GetVolume() { return ResourceDescriptorHeap[srv.volume]; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const VoxelDebug CreateVoxelDebug(VoxelDebug_srv srv)
{
	const VoxelDebug result = {srv
	};
	return result;
}
