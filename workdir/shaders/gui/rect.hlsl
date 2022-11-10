#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
};
#include "../autogen/ColorRect.h"
static const float2 pos[4] = (float2[4])GetColorRect().pos;

#ifdef BUILD_FUNC_VS
quad_output VS(uint index : SV_VERTEXID)
{
    quad_output Output;
    Output.pos = float4(pos[index], 0.99999, 1); //float4(Input.Pos.xy,0.3,1);
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS_COLOR

float4 PS_COLOR(quad_output i) : SV_TARGET0
{
    return GetColorRect().GetColor();
}
#endif

