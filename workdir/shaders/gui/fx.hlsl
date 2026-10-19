// Cursor-driven light inside a rounded rect (Renderer::draw_fx): a soft spot
// at the cursor, a lit rim that is brightest near the cursor, and a diagonal
// sweep. Everything is white light blended over what the element drew, and
// stays inside the shape.

struct vs_out
{
    float4 pos : SV_POSITION;
    float2 wp  : TEXCOORD0;
};

#include "../autogen/UIFx.h"
#include "display_output.hlsl"

#ifdef BUILD_FUNC_VS
vs_out VS(uint index : SV_VERTEXID)
{
    auto d = GetUIFx();

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

float gaussian(float distance, float sigma)
{
    float s = max(sigma, 0.5);
    return exp(-0.5 * distance * distance / (s * s));
}

float4 PS(vs_out i) : SV_TARGET0
{
    auto d = GetUIFx();

    float sd = sd_round_box(i.wp, d.GetShape(), d.GetRadius());
    float inside = saturate(0.5 - sd);
    if (inside <= 0.0)
        discard;

    float cursor_distance = length(i.wp - d.GetReveal().xy);

    // Soft spot at the cursor.
    float light = d.GetReveal().w * gaussian(cursor_distance, d.GetReveal().z);

    // Rim: full at the edge, gone `width` pixels in, scaled by the cursor falloff.
    float width = max(d.GetBorder().z, 0.5);
    float rim = saturate(1.0 + sd / width);
    light += d.GetBorder().y * rim * gaussian(cursor_distance, d.GetBorder().x);

    // Sweep: a soft band moving along a fixed diagonal across the whole shape,
    // from just before its start to just past its end.
    float phase = d.GetShine().x;
    if (phase >= 0.0)
    {
        float2 dir = normalize(float2(1.0, 0.35));
        float2 center = d.GetShape().xy + d.GetShape().zw * 0.5;
        float2 extent = d.GetShape().zw * 0.5;
        float reach = abs(dir.x) * extent.x + abs(dir.y) * extent.y;
        float along = dot(i.wp - center, dir) / max(reach, 1.0);   // -1..1 across the shape
        float band_center = lerp(-1.0 - d.GetShine().y, 1.0 + d.GetShine().y, saturate(phase));
        float t = (along - band_center) / max(d.GetShine().y, 0.01);
        light += d.GetShine().z * exp(-t * t);
    }

    float a = saturate(light) * inside;
    if (a <= 0.0)
        discard;

    return float4(ui_output(float4(1.0, 1.0, 1.0, 1.0)).rgb, a);
}
#endif
