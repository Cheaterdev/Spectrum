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

    const float3 rec709 = float3(0.2126, 0.7152, 0.0722);
    float luma = dot(c, rec709);
    c = lerp(luma.xxx, c, d.GetSaturation());

    float3 tint = ui_output(float4(d.GetTint().rgb, 1.0)).rgb;

    // Acrylic luminosity layer: move the backdrop's luminance toward the
    // tint's while keeping its chroma, so bright and dark backdrops end up at
    // a similar brightness under the glass.
    float lum_now = max(dot(c, rec709), 1e-5);
    float lum_target = lerp(lum_now, dot(tint, rec709), d.GetLuminosity());
    c *= lum_target / lum_now;

    c = lerp(c, tint, d.GetTint().a);

    // Acrylic grain: static per-pixel noise, multiplicative so black stays
    // black (additive grain would lift dark backdrops into grey).
    uint2 px = uint2(i.pos.xy);
    uint h = px.x * 0x8da6b343u ^ px.y * 0xd8163841u;
    h = (h ^ (h >> 16)) * 0x7feb352du;
    h = (h ^ (h >> 15)) * 0x846ca68bu;
    h ^= h >> 16;
    float n = (h & 0xffffu) / 65535.0;
    c *= 1.0 + (n - 0.5) * 2.0 * d.GetNoise();

    return float4(c, 1.0);
}
#endif
