struct vs_out
{
    float4 pos : SV_POSITION;
    float2 wp  : TEXCOORD0;
};

#include "../autogen/BackdropDraw.h"
#include "display_output.hlsl"

#ifdef BUILD_FUNC_VS
vs_out VS(uint index : SV_VERTEXID)
{
    auto d = GetBackdropDraw();

    float2 corner = float2(index & 1, index >> 1);
    float2 wp = d.GetRect().xy + corner * d.GetRect().zw;

    vs_out o;
    o.wp  = wp;
    o.pos = float4(wp.x / d.GetWindow_size().x * 2.0 - 1.0,
                   1.0 - wp.y / d.GetWindow_size().y * 2.0,
                   0.5, 1.0);
    return o;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(vs_out i) : SV_TARGET0
{
    auto d = GetBackdropDraw();

    float2 uv = i.wp * d.GetMap().xy + d.GetMap().zw;

    // The snapshot is a copy of the swapchain, so it is already in output
    // space (scRGB, ui_scale applied); only the tint is authored sRGB.
    float3 c = d.GetBlurred().SampleLevel(linearClampSampler, uv, 0).rgb;

    float luma = dot(c, float3(0.2126, 0.7152, 0.0722));
    c = lerp(luma.xxx, c, d.GetSaturation());

    float3 tint = ui_output(float4(d.GetTint().rgb, 1.0)).rgb;
    c = lerp(c, tint, d.GetTint().a);

    return float4(c, 1.0);
}
#endif
