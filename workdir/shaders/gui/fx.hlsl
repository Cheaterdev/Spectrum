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

    // Inner shadow: darkening just inside the edge, deepest at the top. Drawn
    // first so the light and the focus ring sit on top of it.
    float inner_a = 0.0;
    if (d.GetInner().y > 0.0)
    {
        float sd_shifted = sd_round_box(i.wp - float2(0.0, d.GetInner().z), d.GetShape(), d.GetRadius());
        inner_a = saturate(d.GetInner().y * exp(min(sd_shifted, 0.0) / max(d.GetInner().x, 0.5)));
    }

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

    // Press ripple: a wave leaving the press point, fading as it goes, with a
    // soft fill behind its front.
    float ripple_phase = d.GetRipple().z;
    if (ripple_phase >= 0.0)
    {
        float2 corner_far = max(abs(d.GetRipple().xy - d.GetShape().xy),
                                abs(d.GetRipple().xy - (d.GetShape().xy + d.GetShape().zw)));
        float max_reach = length(corner_far);
        float front = saturate(ripple_phase) * max_reach;
        float ripple_distance = length(i.wp - d.GetRipple().xy);
        float wave = gaussian(ripple_distance - front, 0.12 * max_reach + 4.0);
        float fill = saturate((front - ripple_distance) / (0.4 * front + 1.0));
        light += d.GetRipple().w * (1.0 - ripple_phase) * (wave + 0.35 * fill * (1.0 - ripple_phase));
    }

    // Focus ring: accent-coloured, just inside the edge.
    float ring_a = d.GetFocus().y * saturate(1.0 + sd / max(d.GetFocus().x, 0.5));

    // Over-composite: inner shadow (black) < light (white) < focus ring (accent).
    float light_a = saturate(light);
    float4 under = float4(0.0, 0.0, 0.0, inner_a);
    float4 white = float4(d.GetLight().rgb, light_a);   // the light's own colour (white unless tinted)
    float a_lw = white.a + under.a * (1.0 - white.a);
    float3 c_lw = a_lw > 0.0 ? (white.rgb * white.a + under.rgb * under.a * (1.0 - white.a)) / a_lw : float3(0.0, 0.0, 0.0);

    float3 ring_rgb = d.GetFocus_color().rgb;
    float a_total = ring_a + a_lw * (1.0 - ring_a);
    if (a_total <= 0.0)
        discard;

    float3 srgb = (ring_rgb * ring_a + c_lw * a_lw * (1.0 - ring_a)) / a_total;
    return float4(ui_output(float4(srgb, 1.0)).rgb, saturate(a_total) * inside);
}
#endif
