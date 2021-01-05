#include "autogen/VoxelCopy.h"


static const  RWTexture3D<float4> AlbedoDynamic = GetVoxelCopy().GetTarget(0);
static const Texture3D<float4> AlbedoStatic = GetVoxelCopy().GetSource(0);

static const  RWTexture3D<float4>NormalDynamic = GetVoxelCopy().GetTarget(1);
static const Texture3D<float4>NormalStatic = GetVoxelCopy().GetSource(1);

[numthreads(4, 4, 4)]
void CS(    
	uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex)
{
	uint3 index = GetVoxelCopy().GetParams().get_voxel_pos(dispatchID);

	AlbedoDynamic[index] = AlbedoStatic.Load(uint4(index, 0));
	NormalDynamic[index] =  NormalStatic.Load(uint4(index, 0));
}
