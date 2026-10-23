// How the preview turns a sampled texel into the colour it shows: the R,G,B,A toggles (bit 0..3 of mask),
// exposure, display gamma and the normal-map view. Shared by the 2D, 2D-array and cube draw shaders.

// FrameGraph_Debug_Common.flags (stats.hlsli adds FLAG_CLEAR = 2)
static const uint FLAG_LINEAR  = 4;    // show linear values: no display gamma
static const uint FLAG_CHECKER = 8;    // composite over a checkerboard by the texel's alpha
static const uint FLAG_NORMAL  = 16;   // treat the texel as a tangent-space normal map

// One colour channel left on shows as grey; alpha only shows when no colour channel is on.
float3 apply_channels(float4 v, uint mask)
{
    float3 rgb = max(v.rgb, 0.0);
    uint colorBits = mask & 7u;

    if (colorBits == 0u)
        return (mask & 8u) ? saturate(v.aaa) : float3(0, 0, 0);

    if (countbits(colorBits) == 1u)
    {
        float single = colorBits == 1u ? rgb.r : (colorBits == 2u ? rgb.g : rgb.b);
        return single.xxx;
    }

    return rgb * float3((mask & 1u) != 0u, (mask & 2u) != 0u, (mask & 4u) != 0u);
}

// The colour to write into the preview target, display gamma included.
float3 display_color(float4 v, uint mask, uint flags, float ev)
{
    if (flags & FLAG_NORMAL)
    {
        // x,y from the texel (BC5 / two-channel normal maps carry no z), z rebuilt from them, shown the way a
        // normal map is usually viewed: remapped to 0..1, no exposure and no gamma.
        float3 n;
        n.xy = v.xy * 2.0 - 1.0;
        n.z  = sqrt(saturate(1.0 - dot(n.xy, n.xy)));
        return apply_channels(float4(n * 0.5 + 0.5, v.a), mask);
    }

    float3 c = apply_channels(v, mask) * exp2(ev);
    if ((flags & FLAG_LINEAR) == 0)
        c = pow(c, 1.0 / 2.2);
    return c;
}

// Over a checkerboard by alpha, in display space; the tiles are 8 screen pixels like the area outside the image.
float3 over_checker(float3 color, float alpha, uint2 px)
{
    float3 checker = (((px.x / 8) + (px.y / 8)) % 2 == 0) ? float3(0.30, 0.30, 0.30) : float3(0.55, 0.55, 0.55);
    return lerp(checker, color, saturate(alpha));
}
