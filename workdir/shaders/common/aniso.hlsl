#ifndef ANISO_HLSL
#define ANISO_HLSL

#include "shading_model.hlsl"

// Anisotropic GGX (Burley's parameterisation, as in Filament): the roughness
// along the tangent is widened and along the bitangent narrowed by the
// anisotropy, so highlights stretch perpendicular to the brushing direction.
//
// The engine's direct sun term has no base-layer specular at all (the base
// specular comes from reflections), so for ANISOTROPIC pixels this adds the
// whole sun highlight of the base layer, scaled by pi like the coat/sheen lobes.

void aniso_alphas(float roughness, float anisotropy, out float at, out float ab)
{
    float a = roughness * roughness;
    at = max(a * (1.0 + anisotropy), 0.002);
    ab = max(a * (1.0 - anisotropy), 0.002);
}

float aniso_D_ggx(float NH, float TH, float BH, float at, float ab)
{
    float a2 = at * ab;
    float3 v = float3(ab * TH, at * BH, a2 * NH);
    float v2 = dot(v, v);
    float w2 = a2 / v2;
    return a2 * w2 * w2 / 3.14159265;
}

float aniso_V_smith(float at, float ab, float TV, float BV, float TL, float BL, float NV, float NL)
{
    float lambda_v = NL * length(float3(at * TV, ab * BV, NV));
    float lambda_l = NV * length(float3(at * TL, ab * BL, NL));
    return 0.5 / max(lambda_v + lambda_l, 1e-5);
}

// F0: the base layer's specular colour (dielectric 0.04 blended to albedo by
// metalness); roughness is perceptual, as stored in the G-buffer.
float3 aniso_sun_specular(float4 material, float3 N, float3 V, float3 L, float shadow, float3 F0, float roughness)
{
    if (material_model(material) != SHADING_MODEL_ANISOTROPIC)
        return 0;

    float anisotropy = material_params(material).x;
    float3 T = aniso_decode_tangent(N, material_params(material).y);
    float3 B = cross(N, T);

    float at, ab;
    aniso_alphas(roughness, anisotropy, at, ab);

    float NL = saturate(dot(N, L));
    float NV = max(dot(N, V), 1e-4);

    if (NL <= 0.0)
        return 0;

    float3 H = normalize(V + L);
    float NH = saturate(dot(N, H));
    float VH = saturate(dot(V, H));

    float3 F = F0 + (1.0 - F0) * pow(1.0 - VH, 5.0);
    float spec = aniso_D_ggx(NH, dot(T, H), dot(B, H), at, ab)
               * aniso_V_smith(at, ab, dot(T, V), dot(B, V), dot(T, L), dot(B, L), NV, NL);

    return shadow * F * spec * NL * 3.14159265;
}

#endif
