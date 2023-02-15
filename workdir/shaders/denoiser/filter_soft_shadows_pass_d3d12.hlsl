/**********************************************************************
Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. 
RTShadowDenoiser - Filter Pass 0
********************************************************************/

#include "../autogen/DenoiserShadow_Filter.h"
#if Pass!=2
#include "../autogen/DenoiserShadow_FilterLocal.h"


float16_t2 FFX_DNSR_Shadows_ReadInput(int2 p)
{
    return (float16_t2)GetDenoiserShadow_FilterLocal().GetRqt2d_input().Load(int3(p, 0)).xy;
}
#else
#include "../autogen/DenoiserShadow_FilterLast.h"


float16_t2 FFX_DNSR_Shadows_ReadInput(int2 p)
{
    return (float16_t2)GetDenoiserShadow_FilterLast().GetRqt2d_input().Load(int3(p, 0)).xy;
}
#endif

float2 FFX_DNSR_Shadows_GetInvBufferDimensions()
{
    return GetDenoiserShadow_Filter().InvBufferDimensions;
}

int2 FFX_DNSR_Shadows_GetBufferDimensions()
{
    return GetDenoiserShadow_Filter().BufferDimensions;
}

float4x4 FFX_DNSR_Shadows_GetProjectionInverse()
{
    return GetDenoiserShadow_Filter().ProjectionInverse;
}

float FFX_DNSR_Shadows_GetDepthSimilaritySigma()
{
    return 0.1;//GetDenoiserShadow_Filter().DepthSimilaritySigma;
}

float FFX_DNSR_Shadows_ReadDepth(int2 p)
{
    return GetDenoiserShadow_Filter().GetT2d_DepthBuffer().Load(int3(p, 0));
}

float16_t3 FFX_DNSR_Shadows_ReadNormals(int2 p)
{
    return normalize(((float16_t3)GetDenoiserShadow_Filter().GetT2d_NormalBuffer().Load(int3(p, 0))).xyz * 2 - 1.f);
}

bool FFX_DNSR_Shadows_IsShadowReciever(uint2 p)
{
    float depth = FFX_DNSR_Shadows_ReadDepth(p);
    return (depth > 0.0f) && (depth < 1.0f);
}


uint FFX_DNSR_Shadows_ReadTileMetaData(uint p)
{
    return GetDenoiserShadow_Filter().GetSb_tileMetaData()[p];
}

#include "ffx-shadows-dnsr/ffx_denoiser_shadows_filter.h"
#if Pass == 0
[numthreads(8, 8, 1)]
void main(uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint2 did : SV_DispatchThreadID)
{
    const uint PASS_INDEX = 0;
    const uint STEP_SIZE = 1;

    bool bWriteOutput = false;
    float2 const results = FFX_DNSR_Shadows_FilterSoftShadowsPass(gid, gtid, did, bWriteOutput, PASS_INDEX, STEP_SIZE);

    if (bWriteOutput)
    {
        GetDenoiserShadow_FilterLocal().GetRwt2d_output()[did] = results;
    }
}
#elif Pass == 1
[numthreads(8, 8, 1)]
void main(uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint2 did : SV_DispatchThreadID)
{
    const uint PASS_INDEX = 1;
    const uint STEP_SIZE = 2;

    bool bWriteOutput = false;
    float2 const results = FFX_DNSR_Shadows_FilterSoftShadowsPass(gid, gtid, did, bWriteOutput, PASS_INDEX, STEP_SIZE);
    if (bWriteOutput)
    {
         GetDenoiserShadow_FilterLocal().GetRwt2d_output()[did] = results;
    }
}
#elif Pass == 2

float ShadowContrastRemapping(float x)
{
    const float a = 10.f;
    const float b = -1.0f;
    const float c = 1 / pow(2, a);
    const float d = exp(-b);
    const float e = 1 / (1 / pow((1 + d), a) - c);
    const float m = 1 / pow((1 + pow(d, x)), a) - c;

    return m * e;
}

[numthreads(8, 8, 1)]
void main(uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint2 did : SV_DispatchThreadID)
{
    const uint PASS_INDEX = 2;
    const uint STEP_SIZE = 2;

    bool bWriteOutput = false;
    float2 const results = FFX_DNSR_Shadows_FilterSoftShadowsPass(gid, gtid, did, bWriteOutput, PASS_INDEX, STEP_SIZE);

    // Recover some of the contrast lost during denoising
    const float shadow_remap = max(1.3f - results.y, 1.0f);
    const float mean = pow(abs(results.x), shadow_remap);

    if (bWriteOutput)
    {
         GetDenoiserShadow_FilterLast().GetRwt2d_output()[did] = mean;
    }
}
#endif