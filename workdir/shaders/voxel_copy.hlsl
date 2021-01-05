#include "autogen/VoxelCopy.h"


static const  RWTexture3D<float4> AlbedoDynamic = GetVoxelCopy().GetTarget(0);
static const Texture3D<float4> AlbedoStatic = GetVoxelCopy().GetSource(0);

static const  RWTexture3D<float4>NormalDynamic = GetVoxelCopy().GetTarget(1);
static const Texture3D<float4>NormalStatic = GetVoxelCopy().GetSource(1);

static const StructuredBuffer<int3> voxel_visibility = GetVoxelCopy().GetVisibility();
static const uint group_count = GetVoxelCopy().GetGroupCount();

uint3 get_index(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / GetVoxelCopy().GetVoxels_per_tile().x;
	uint3 tile_pos = voxel_visibility[tile_index] * GetVoxelCopy().GetVoxels_per_tile().xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index* GetVoxelCopy().GetVoxels_per_tile().x,0,0);
	uint3 index = tile_pos + tile_local_pos;
	return index;
}

[numthreads(4, 4, 4)]
void CS(    
	uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex)
{
	uint3 index = get_index(dispatchID);

	AlbedoDynamic[index] = AlbedoStatic.Load(uint4(index, 0));
	NormalDynamic[index] =  NormalStatic.Load(uint4(index, 0));


}
