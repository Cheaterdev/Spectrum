// Applies the preview's R,G,B,A toggles (bit 0..3 of mask) to a sampled texel.
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
