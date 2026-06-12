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
    vertex_input v = GetNinePatch().GetVb()[index + instance * 16];
    quad_output Output;
    Output.pos            = float4(v.pos, 0.5, 1);
    Output.tc             = v.tc;
    Output.mulColor       = v.mulColor;
    Output.addColor       = v.addColor;
    Output.texture_offset = (float)instance;
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(quad_output i) : SV_TARGET0
{
    Texture2D<float4> tex = GetNinePatch().GetTextures((int)i.texture_offset);
    float4 color = tex.Sample(linearSampler, i.tc);
    return color * i.mulColor + i.addColor;
}
#endif

