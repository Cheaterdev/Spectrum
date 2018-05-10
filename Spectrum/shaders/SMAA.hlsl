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
/**
 * This can be ignored; its purpose is to support interactive custom parameter
 * tweaking.
 */
cbuffer sizes: register(b0)
{
    float4 subsampleIndices;
    float4 SMAA_RT_METRICS;
};
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

// And include our header!
#include "SMAA_impl.hlsl"

// Set pixel shader version accordingly:
#if SMAA_HLSL_4_1
#define PS_VERSION ps_4_1
#else
#define PS_VERSION ps_4_0
#endif


/**
 * DepthStencilState's and company
 */
DepthStencilState DisableDepthStencil
{
    DepthEnable = FALSE;
    StencilEnable = FALSE;
};

DepthStencilState DisableDepthReplaceStencil
{
    DepthEnable = FALSE;
    StencilEnable = TRUE;
    FrontFaceStencilPass = REPLACE;
};

DepthStencilState DisableDepthUseStencil
{
    DepthEnable = FALSE;
    StencilEnable = TRUE;
    FrontFaceStencilFunc = EQUAL;
};

BlendState Blend
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = BLEND_FACTOR;
    DestBlend = INV_BLEND_FACTOR;
    BlendOp = ADD;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};


/**
* Pre-computed area and search textures
*/
Texture2D areaTex: register(t0);
Texture2D searchTex: register(t1);


/**
* Temporal textures
*/
Texture2D edgesTex: register(t2);
Texture2D blendTex: register(t3);


/**
 * Input textures
 */
//Texture2D colorTexGamma;
//Texture2D colorTexPrev;
//Texture2DMS<float4, 2> colorTexMS;
Texture2D depthTex: register(t4);

Texture2D colorTex: register(t5);
Texture2D colorTexPrev : register(t6);
//Texture2D velocityTex;




struct quad_output
{
float4 pos : SV_POSITION;
float2 tc : TEXCOORD0;
};

quad_output VS(uint index : SV_VERTEXID)
{
    static float2 Pos[] =
    {
        float2(-1, 1),
        float2(1, 1),
        float2(-1, -1),
        float2(1, -1)
    };
    static float2 Tex[] =
    {

        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1),
    };
    quad_output Output;
    Output.pos = float4(Pos[index], 0.3, 1);
    Output.tc = Tex[index];
    return Output;
}


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

float2 DX10_SMAALumaEdgeDetectionPS(float4 position : SV_POSITION,
                                    float2 texcoord : TEXCOORD0,
                                    float4 offset[3] : TEXCOORD1) : SV_TARGET
{
#if SMAA_PREDICATION
    return SMAALumaEdgeDetectionPS(texcoord, offset, colorTex, depthTex);
#else
    return SMAALumaEdgeDetectionPS(texcoord, offset, colorTex);
#endif
}
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
float4 DX10_SMAABlendingWeightCalculationPS(float4 position : SV_POSITION,
        float2 texcoord : TEXCOORD0,
        float2 pixcoord : TEXCOORD1,
        float4 offset[3] : TEXCOORD2) : SV_TARGET
{
    return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edgesTex, areaTex, searchTex, subsampleIndices);
}

float4 DX10_SMAANeighborhoodBlendingPS(float4 position : SV_POSITION,
                                       float2 texcoord : TEXCOORD0,
                                       float4 offset : TEXCOORD1) : SV_TARGET
{
#if SMAA_REPROJECTION
    return SMAANeighborhoodBlendingPS(texcoord, offset, colorTex, blendTex, velocityTex);
#else
    return SMAANeighborhoodBlendingPS(texcoord, offset, colorTex, blendTex);
#endif
}


float4 DX10_SMAAResolvePS(float4 position : SV_POSITION,
                          float2 texcoord : TEXCOORD0) : SV_TARGET
{
#if SMAA_REPROJECTION
    return SMAAResolvePS(texcoord, colorTex, colorTexPrev, velocityTex);
#else
    return SMAAResolvePS(texcoord, colorTex, colorTexPrev);
#endif
}
/*
void DX10_SMAASeparatePS(float4 position : SV_POSITION,
                         float2 texcoord : TEXCOORD0,
                         out float4 target0 : SV_TARGET0,
                         out float4 target1 : SV_TARGET1)
{
    SMAASeparatePS(position, texcoord, target0, target1, colorTexMS);
}

*/