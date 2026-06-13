#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float4 mulColor:TEXCOORD0;
float4 addColor:TEXCOORD1;
float2 tc: TEXCOORD2;
float texture_offset : TEXCOORD3;
};


#include "../autogen/NinePatch.h"

#ifdef BUILD_FUNC_VS
quad_output VS(uint index : SV_VERTEXID, uint instance : SV_INSTANCEID)
{
    // DIAGNOSTIC: hardcoded fullscreen triangle — bypasses all VB/bindless reads.
    // If this produces visible output, the VB bindless read is the failure point.
    float2 positions[3] = { float2(-1,-1), float2(3,-1), float2(-1,3) };
    quad_output Output;
    Output.pos            = float4(positions[index % 3], 0.5, 1);
    Output.tc             = float2(0,0);
    Output.mulColor       = float4(1,1,1,1);
    Output.addColor       = float4(0,0,0,0);
    Output.texture_offset = 0;
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(quad_output i) : SV_TARGET0
{
    // DIAGNOSTIC: return solid red to check if draw pipeline itself works
    return float4(1, 0, 0, 1);
}
#endif

