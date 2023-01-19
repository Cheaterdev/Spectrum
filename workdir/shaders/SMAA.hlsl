/**
 * Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. As clarification, there
 * is no requirement that the copyright notice and permission be included in
 * binary distributions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "2D_screen_simple.h"

#define SMAA_PRESET_ULTRA 1
#define SMAA_HLSL_4_1 1
/**
 * This is only required for temporal modes (SMAA T2x).
 */
//static const float4 subsampleIndices = 0;

/**
 * This is required for blending the results of previous subsample with the
 * output render target; it's used in SMAA S2x and 4x, for other modes just use
 * 1.0 (no blending).
 */
//float blendFactor;
static const float blendFactor = 0;

/*

// Use a real macro here for maximum performance!
#ifndef SMAA_RT_METRICS // This is just for compilation-time syntax checking.
#define SMAA_RT_METRICS float4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
#endif
*/
// Set the HLSL version:
#ifndef SMAA_HLSL_4_1
#define SMAA_HLSL_4
#endif

// Set preset defines:
#ifdef SMAA_PRESET_CUSTOM
float threshld;
float maxSearchSteps;
float maxSearchStepsDiag;
float cornerRounding;

#define SMAA_THRESHOLD threshld
#define SMAA_MAX_SEARCH_STEPS maxSearchSteps
#define SMAA_MAX_SEARCH_STEPS_DIAG maxSearchStepsDiag
#define SMAA_CORNER_ROUNDING cornerRounding
#endif


#include "autogen/SMAA_Global.h"

static const float4 subsampleIndices = GetSMAA_Global().GetSubsampleIndices();
static const float4 SMAA_RT_METRICS = GetSMAA_Global().GetSMAA_RT_METRICS();
// And include our header!
#include "SMAA_impl.hlsl"

// Set pixel shader version accordingly:
#if SMAA_HLSL_4_1
#define PS_VERSION ps_4_1
#else
#define PS_VERSION ps_4_0
#endif


/**
 * Function wrappers
 */
void DX10_SMAAEdgeDetectionVS(uint index : SV_VERTEXID,
                              out float4 svPosition : SV_POSITION,
                              out float2 texcoord : TEXCOORD0,
                              out float4 offset[3] : TEXCOORD1)
{
    quad_output o = VS(index);
    svPosition = o.pos;
    texcoord = o.tc;
    SMAAEdgeDetectionVS(texcoord, offset);
}

void DX10_SMAABlendingWeightCalculationVS(uint index : SV_VERTEXID,
        out float4 svPosition : SV_POSITION,
        out float2 texcoord : TEXCOORD0,
        out float2 pixcoord : TEXCOORD1,
        out float4 offset[3] : TEXCOORD2)
{
    quad_output o = VS(index);
    svPosition = o.pos;
    texcoord = o.tc;
    SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
}

void DX10_SMAANeighborhoodBlendingVS(uint index : SV_VERTEXID,
                                     out float4 svPosition : SV_POSITION,
                                     out float2 texcoord : TEXCOORD0,
                                     out float4 offset : TEXCOORD1)
{
    quad_output o = VS(index);
    svPosition = o.pos;
    texcoord = o.tc;
    SMAANeighborhoodBlendingVS(texcoord, offset);
}

void DX10_SMAAResolveVS(float4 position : POSITION,
                        out float4 svPosition : SV_POSITION,
                        inout float2 texcoord : TEXCOORD0)
{
    svPosition = position;
}

void DX10_SMAASeparateVS(float4 position : POSITION,
                         out float4 svPosition : SV_POSITION,
                         inout float2 texcoord : TEXCOORD0)
{
    svPosition = position;
}


#ifdef BUILD_FUNC_DX10_SMAALumaEdgeDetectionPS

    float2 DX10_SMAALumaEdgeDetectionPS(float4 position : SV_POSITION,
                                        float2 texcoord : TEXCOORD0,
                                        float4 offset[3] : TEXCOORD1) : SV_TARGET
    {
    #if SMAA_PREDICATION
        return SMAALumaEdgeDetectionPS(texcoord, offset, GetSMAA_Global().GetColorTex(), depthTex);
    #else
        return SMAALumaEdgeDetectionPS(texcoord, offset, GetSMAA_Global().GetColorTex());
    #endif
    }
#endif
/*
float2 DX10_SMAAColorEdgeDetectionPS(float4 position : SV_POSITION,
                                     float2 texcoord : TEXCOORD0,
                                     float4 offset[3] : TEXCOORD1) : SV_TARGET
{
#if SMAA_PREDICATION
    return SMAAColorEdgeDetectionPS(texcoord, offset, colorTexGamma, depthTex);
#else
    return SMAAColorEdgeDetectionPS(texcoord, offset, colorTexGamma);
#endif
}
*/

/*
float2 DX10_SMAADepthEdgeDetectionPS(float4 position : SV_POSITION,
                                     float2 texcoord : TEXCOORD0,
                                     float4 offset[3] : TEXCOORD1) : SV_TARGET
{
    return SMAADepthEdgeDetectionPS(texcoord, offset, depthTex);
}
*/


#ifdef BUILD_FUNC_DX10_SMAABlendingWeightCalculationPS


#include "autogen/SMAA_Weights.h"

    float4 DX10_SMAABlendingWeightCalculationPS(float4 position : SV_POSITION,
            float2 texcoord : TEXCOORD0,
            float2 pixcoord : TEXCOORD1,
            float4 offset[3] : TEXCOORD2) : SV_TARGET
    {
        return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, GetSMAA_Weights().GetEdgesTex(), GetSMAA_Weights().GetAreaTex(), GetSMAA_Weights().GetSearchTex(), GetSMAA_Global().GetSubsampleIndices());
    }

#endif


#ifdef BUILD_FUNC_DX10_SMAANeighborhoodBlendingPS
#include "autogen/SMAA_Blend.h"

        float4 DX10_SMAANeighborhoodBlendingPS(float4 position : SV_POSITION,
            float2 texcoord : TEXCOORD0,
            float4 offset : TEXCOORD1) : SV_TARGET
    {
    #if SMAA_REPROJECTION
        return 1;// SMAANeighborhoodBlendingPS(texcoord, offset, GetSMAA_Global().GetColorTex(), GetSMAA_Blend().GetBlendTex(), velocityTex);
    #else
        return pow(SMAANeighborhoodBlendingPS(texcoord, offset, GetSMAA_Global().GetColorTex(), GetSMAA_Blend().GetBlendTex()),1.0/2.2);
    #endif
    }
#endif
