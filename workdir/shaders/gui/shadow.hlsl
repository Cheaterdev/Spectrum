// Procedural soft shadow / glow around a rounded rect (Renderer::draw_shadow).
// A rounded box blurred by a Gaussian is approximated by applying the 1D
// Gaussian CDF to the box's signed distance -- exact for straight edges, close
// for the corners at UI sizes.

struct vs_out
{
    float4 pos : SV_POSITION;
    float2 wp  : TEXCOORD0;
};

#include "../autogen/UIShadow.h"
#include "display_output.hlsl"

#ifdef BUILD_FUNC_VS
vs_out VS(uint index : SV_VERTEXID)
{
    auto d = GetUIShadow();

    float2 corner = float2(index & 1, index >> 1);
    float2 wp = d.GetQuad().xy + corner * d.GetQuad().zw;

    vs_out o;
    o.wp  = wp;
    o.pos = float4(wp.x / d.GetWindow_size().x * 2.0 - 1.0,
                   1.0 - wp.y / d.GetWindow_size().y * 2.0,
                   0.5, 1.0);
    return o;
}
#endif

#ifdef BUILD_FUNC_PS
float sd_round_box(float2 p, float4 rect, float radius)
{
    float2 half_size = rect.zw * 0.5;
    float r = min(radius, min(half_size.x, half_size.y));
    float2 q = abs(p - (rect.xy + half_size)) - half_size + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

// Abramowitz & Stegun 7.1.26-style approximation (max error ~1e-4), enough
// for an 8/16-bit alpha ramp.
float erf_approx(float x)
{
    float a = 0.147;
    float x2 = x * x;
    float e = sqrt(1.0 - exp(-x2 * (4.0 / 3.14159265 + a * x2) / (1.0 + a * x2)));
    return x < 0 ? -e : e;
}

float4 PS(vs_out i) : SV_TARGET0
{
    auto d = GetUIShadow();

    float dist = sd_round_box(i.wp, d.GetShape(), d.GetRadius());
    float coverage = 0.5 - 0.5 * erf_approx(dist / (d.GetSigma() * 1.41421356));

    // Outside the element only: under a translucent element a shadow would
    // darken it. One pixel of ramp keeps the element's edge antialiased.
    float outside = saturate(sd_round_box(i.wp, d.GetElement(), d.GetRadius()) + 0.5);

    float a = d.GetColor().a * coverage * outside;
    if (a <= 0.0)
        discard;

    return float4(ui_output(float4(d.GetColor().rgb, 1.0)).rgb, a);
}
#endif
