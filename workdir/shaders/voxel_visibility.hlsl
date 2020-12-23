//
// Copyright (c) Microsoft. All rights reserved.

#include "autogen/VoxelVisibility.h"

[numthreads(4, 4, 4)]
void CS(    
	uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex)
{

    uint v = GetVoxelVisibility().GetVisibility()[dispatchID];

    if(v>0)
    GetVoxelVisibility().GetVisible_tiles().Append(uint4(dispatchID, 0));

}
