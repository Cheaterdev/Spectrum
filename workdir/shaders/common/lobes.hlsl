#ifndef LOBES_HLSL
#define LOBES_HLSL

#include "coat.hlsl"
#include "sheen.hlsl"
#include "aniso.hlsl"
#include "iridescence.hlsl"

// The base layer's own sun highlight for metals. The engine's direct term is
// Lambert-only (shadow * NL * albedo * (1 - metallic)), so a metal -- which has no
// diffuse -- received no sun light at all and showed only what its reflection ray
// found. This adds the GGX highlight a metal needs (F0 = its base colour),
// weighted by metalness: dielectrics keep the look they had, since their specular
// comes from reflections. Anisotropic pixels have their own base highlight.
float3 base_sun_specular(float4 material, float3 N, float3 V, float3 L, float shadow, float3 F0, float roughness, float metallic)
{
    if (metallic <= 0.001 || material_model(material) == SHADING_MODEL_ANISOTROPIC)
        return 0;

    float a = max(roughness * roughness, 0.002);
    float NL = saturate(dot(N, L));
    float NV = max(dot(N, V), 1e-4);

    if (NL <= 0.0)
        return 0;

    float3 H = normalize(V + L);
    float NH = saturate(dot(N, H));
    float VH = saturate(dot(V, H));

    float3 F = F0 + (1.0 - F0) * pow(1.0 - VH, 5.0);

    if (material_model(material) == SHADING_MODEL_THINFILM)
        F = iridescent_fresnel(VH, thinfilm_thickness(material), thinfilm_ior(material), F0);

    return metallic * shadow * F * (coat_D_ggx(NH, a) * coat_V_smith(NL, NV, a) * NL * 3.14159265);
}

// Every extra specular lobe a material's shading model adds to the direct sun
// term, behind one call so the lighting passes do not each list them.
//
// F0 and the perceptual roughness are the base layer's (G-buffer albedo/metallic
// and the normals alpha); only the anisotropic model reads them, since it is the
// one that replaces the base specular instead of adding a layer above it.
float3 material_sun_lobes(float4 material, float3 N, float3 V, float3 L, float shadow, float3 F0, float roughness, float3 albedo, float metallic)
{
    return coat_sun_specular(material, N, V, L, shadow)
         + sheen_sun_specular(material, N, V, L, shadow, albedo)
         + aniso_sun_specular(material, N, V, L, shadow, F0, roughness)
         + base_sun_specular(material, N, V, L, shadow, F0, roughness, metallic);
}

// Without a metalness: no base highlight.
float3 material_sun_lobes(float4 material, float3 N, float3 V, float3 L, float shadow, float3 F0, float roughness, float3 albedo)
{
    return material_sun_lobes(material, N, V, L, shadow, F0, roughness, albedo, 0.0);
}

// Without a base colour: white sheen.
float3 material_sun_lobes(float4 material, float3 N, float3 V, float3 L, float shadow, float3 F0, float roughness)
{
    return material_sun_lobes(material, N, V, L, shadow, F0, roughness, 1.0);
}

#endif
