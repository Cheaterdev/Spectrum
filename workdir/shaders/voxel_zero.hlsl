#include "autogen/VoxelZero.h"

static const RWTexture3D<float4> Target = GetVoxelZero().GetTarget();

[numthreads(4, 4, 4)]
void CS(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{
	uint3 index = GetVoxelZero().GetParams().get_voxel_pos(dispatchID);

	Target[index] = 0;
}
