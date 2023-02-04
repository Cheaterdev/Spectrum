#define TILE_SIZE_X 8
#define TILE_SIZE_Y 4

#include "../autogen/DenoiserShadow_Prepare.h"

uint LaneIdToBitShift(uint2 localID)
{
    return localID.y * TILE_SIZE_X + localID.x;
}

bool WaveMaskToBool(uint mask, uint2 localID)
{
    return (1 << LaneIdToBitShift(localID.xy)) & mask;
}

int2 FFX_DNSR_Shadows_GetBufferDimensions()
{
    return GetDenoiserShadow_Prepare().GetBufferDimensions();
}

bool FFX_DNSR_Shadows_HitsLight(uint2 did, uint2 gtid, uint2 gid)
{
    return GetDenoiserShadow_Prepare().GetT2d_hitMaskResults()[did];//>0.6;//!WaveMaskToBool(GetDenoiserShadow_Prepare().GetT2d_hitMaskResults()[gid*uint2(8,4)]<0.6, gtid);
}

void FFX_DNSR_Shadows_WriteMask(uint offset, uint value)
{
    GetDenoiserShadow_Prepare().GetRwsb_shadowMask()[offset] = value;
} 

#include "ffx-shadows-dnsr/ffx_denoiser_shadows_prepare.h"

[numthreads(TILE_SIZE_X, TILE_SIZE_Y, 1)]
void main(uint2 gtid : SV_GroupThreadID, uint2 gid : SV_GroupID)
{
    FFX_DNSR_Shadows_PrepareShadowMask(gtid, gid);
}
