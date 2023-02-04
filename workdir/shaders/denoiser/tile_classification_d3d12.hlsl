/**********************************************************************
Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
RTShadowDenoiser

tile_classification

********************************************************************/

#include "../autogen/DenoiserShadow_TileClassification.h"


float4x4 FFX_DNSR_Shadows_GetViewProjectionInverse()
{
    return GetDenoiserShadow_TileClassification().ViewProjectionInverse;
}

float4x4 FFX_DNSR_Shadows_GetReprojectionMatrix()
{
    return GetDenoiserShadow_TileClassification().ReprojectionMatrix;
}

float4x4 FFX_DNSR_Shadows_GetProjectionInverse()
{
    return GetDenoiserShadow_TileClassification().ProjectionInverse;
}

float2 FFX_DNSR_Shadows_GetInvBufferDimensions()
{
    return GetDenoiserShadow_TileClassification().InvBufferDimensions;
}

int2 FFX_DNSR_Shadows_GetBufferDimensions()
{
    return GetDenoiserShadow_TileClassification().BufferDimensions;
}

int FFX_DNSR_Shadows_IsFirstFrame()
{
    return GetDenoiserShadow_TileClassification().FirstFrame;
}

float3 FFX_DNSR_Shadows_GetEye()
{
    return GetDenoiserShadow_TileClassification().Eye;
}

float FFX_DNSR_Shadows_ReadDepth(int2 p)
{
    return GetDenoiserShadow_TileClassification().GetT2d_depth().Load(int3(p, 0)).x;
}

float FFX_DNSR_Shadows_ReadPreviousDepth(int2 p)
{
    return GetDenoiserShadow_TileClassification().GetT2d_previousDepth().Load(int3(p, 0)).x;
} 

float3 FFX_DNSR_Shadows_ReadNormals(int2 p)
{
    return normalize(GetDenoiserShadow_TileClassification().GetT2d_normal().Load(int3(p, 0)).xyz * 2 - 1.f);
}

float2 FFX_DNSR_Shadows_ReadVelocity(int2 p)
{
    return GetDenoiserShadow_TileClassification().GetT2d_velocity().Load(int3(p, 0))*float2(2,-2);
}

float FFX_DNSR_Shadows_ReadHistory(float2 p)
{
    return GetDenoiserShadow_TileClassification().GetT2d_history().SampleLevel(linearClampSampler, p, 0).x;
}

float3 FFX_DNSR_Shadows_ReadPreviousMomentsBuffer(int2 p)
{
    return GetDenoiserShadow_TileClassification().GetT2d_previousMoments().Load(int3(p, 0)).xyz;
}

uint  FFX_DNSR_Shadows_ReadRaytracedShadowMask(uint p)
{
    return GetDenoiserShadow_TileClassification().GetSb_raytracerResult()[p];
}

void  FFX_DNSR_Shadows_WriteMetadata(uint p, uint val)
{
    GetDenoiserShadow_TileClassification().GetRwsb_tileMetaData()[p] = val;
}

void  FFX_DNSR_Shadows_WriteMoments(uint2 p, float3 val)
{
    GetDenoiserShadow_TileClassification().GetRwt2d_momentsBuffer()[p] = val;
}

void FFX_DNSR_Shadows_WriteReprojectionResults(uint2 p, float2 val)
{
    GetDenoiserShadow_TileClassification().GetRwt2d_reprojectionResults()[p] = val;
}

bool FFX_DNSR_Shadows_IsShadowReciever(uint2 p)
{
    float depth = FFX_DNSR_Shadows_ReadDepth(p);
    return (depth > 0.0f) && (depth < 1.0f);
}

#include "ffx-shadows-dnsr/ffx_denoiser_shadows_tileclassification.h"

[numthreads(8, 8, 1)]
void main(uint group_index : SV_GroupIndex, uint2 gid : SV_GroupID)
{
    FFX_DNSR_Shadows_TileClassification(group_index, gid);
}
