#ifndef SHEEN_HLSL
#define SHEEN_HLSL

#include "shading_model.hlsl"

// Sheen lobe for direct sun light: the microfiber ("Charlie") distribution of
// Estevez & Kulla with Neubelt's visibility, as in Filament. It peaks at
// grazing angles, which is what gives cloth its soft rim. Scaled by pi like
// the coat lobe (the engine's direct term is Lambert without 1/pi).
//
// Not modelled: sheen colour (white), and the base layer being darkened by the
// sheen's directional albedo -- a few percent at typical weights.
//
// Indirect light and reflections get no sheen term: it is a broad lobe, and
// the indirect passes have no per-lobe input to put it in.

float sheen_D_charlie(float NH, float a)
{
    float inv_a = 1.0 / a;
    float sin2h = max(1.0 - NH * NH, 0.0078125);   // keeps the pow() finite at NH = 1
    return (2.0 + inv_a) * pow(sin2h, inv_a * 0.5) / (2.0 * 3.14159265);
}

float sheen_V_neubelt(float NL, float NV)
{
    return 1.0 / (4.0 * (NL + NV - NL * NV));
}

float3 sheen_sun_specular(float4 material, float3 N, float3 V, float3 L, float shadow)
{
    if (material_model(material) != SHADING_MODEL_SHEEN)
        return 0;

    float weight = material_params(material).x;
    // Perceptual roughness squared, floored: a tiny alpha makes pow() underflow.
    float a = max(material_params(material).y * material_params(material).y, 0.02);

    float NL = saturate(dot(N, L));
    float NV = max(dot(N, V), 1e-4);

    if (NL <= 0.0)
        return 0;

    float NH = saturate(dot(N, normalize(V + L)));

    return shadow * weight * sheen_D_charlie(NH, a) * sheen_V_neubelt(NL, NV) * NL * 3.14159265;
}

#endif
