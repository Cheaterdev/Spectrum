#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float4 mulColor:TEXCOORD0;
float4 addColor:TEXCOORD1;
float2 tc: TEXCOORD2;
nointerpolation uint texture_offset : TEXCOORD3;
nointerpolation float gammaEncode : TEXCOORD4;
};


#include "../autogen/NinePatch.h"
static const StructuredBuffer<vertex_input> vb = GetNinePatch().GetVb();

#ifdef BUILD_FUNC_VS
quad_output VS(uint index : SV_VERTEXID, uint instance : SV_INSTANCEID)
{

	vertex_input input = vb[16 * instance + index];
    quad_output Output;
    Output.pos = float4(input.GetPos(), 0.999, 1);
    Output.tc = input.GetTc();
	Output.texture_offset = instance;// texture_offset[instance];
    Output.mulColor = input.GetMulColor();
    Output.addColor = input.GetAddColor();
    Output.gammaEncode = input.GetGammaEncode();

    return Output;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(quad_output i) : SV_TARGET0
{
    float4 col = GetNinePatch().GetTextures(i.texture_offset).Sample(anisoBordeSampler , i.tc);
    //col.xyz/=col.w;
    float4 result = i.addColor + i.mulColor * col;
    if (i.gammaEncode > 0.5)
        result.rgb = pow(result.rgb, 1.0 / 2.2);
    return result;
}
#endif
