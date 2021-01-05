#include "autogen/VoxelZero.h"

static const  RWTexture3D<float4> Target = GetVoxelZero().GetTarget();
static const StructuredBuffer<int3> voxel_visibility = GetVoxelZero().GetVisibility();
static const uint group_count = GetVoxelZero().GetGroupCount();

uint3 get_index(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / GetVoxelZero().GetVoxels_per_tile().x;
	uint3 tile_pos = voxel_visibility[tile_index] * GetVoxelZero().GetVoxels_per_tile().xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index * GetVoxelZero().GetVoxels_per_tile().x, 0, 0);
	uint3 index = tile_pos + tile_local_pos;
	return index;
}

[numthreads(4, 4, 4)]
void CS(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{
	uint local_index = groupThreadID.x + groupThreadID.y * 4 + groupThreadID.z * 16;
	uint3 index = get_index(dispatchID);

	Target[index] = 0;
}
