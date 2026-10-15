#ifndef COAT_HLSL
#define COAT_HLSL

#include "shading_model.hlsl"

// Clearcoat lobe for direct sun light: a second, dielectric GGX specular layer
// over the base. The engine's direct term is Lambert-only (shadow * NL *
// albedo, no 1/pi -- i.e. sun irradiance is taken as pi), so the specular term
// is scaled by pi to sit in the same units.
//
// Not modelled: the coat darkening the base layer below it, and the coat's own
// refraction of the view/light rays. At the coat's fixed IOR 1.5 the darkening
// is a few percent, which is below what the rest of the lighting resolves.

#define COAT_F0 0.04   // dielectric, IOR 1.5

float coat_D_ggx(float NH, float a)
{
    float a2 = a * a;
    float d = NH * NH * (a2 - 1.0) + 1.0;
    return a2 / (3.14159265 * d * d);
}

// Height-correlated Smith visibility, already divided by 4 NL NV.
float coat_V_smith(float NL, float NV, float a)
{
    float a2 = a * a;
    float gv = NL * sqrt(NV * NV * (1.0 - a2) + a2);
    float gl = NV * sqrt(NL * NL * (1.0 - a2) + a2);
    return 0.5 / max(gv + gl, 1e-5);
}

// Mirror-direction radiance seen through the coat: the coat's Fresnel reflectance
// (Schlick, from the view angle) times its weight. `radiance` comes from a ray
// traced at the coat's own roughness (ReflectionRTX's coat output).
float3 coat_reflection(float4 material, float3 N, float3 V, float3 radiance)
{
    if (material_model(material) != SHADING_MODEL_CLEARCOAT)
        return 0;

    float NV = saturate(dot(N, V));
    float F = COAT_F0 + (1.0 - COAT_F0) * pow(1.0 - NV, 5.0);
    return material_params(material).x * F * radiance;
}

// What reaches the base layer under a clearcoat: the coat reflects F of the
// light on the way in. The way out is left out, at IOR 1.5 it is a few
// percent. 1 for materials without a coat.
float coat_base_attenuation(float4 material, float3 N, float3 V)
{
    if (material_model(material) != SHADING_MODEL_CLEARCOAT)
        return 1.0;

    float NV = saturate(dot(N, V));
    float F = COAT_F0 + (1.0 - COAT_F0) * pow(1.0 - NV, 5.0);
    return 1.0 - material_params(material).x * F;
}

float3 coat_sun_specular(float4 material, float3 N, float3 V, float3 L, float shadow)
{
    if (material_model(material) != SHADING_MODEL_CLEARCOAT)
        return 0;

    float weight = material_params(material).x;
    // Perceptual roughness squared; floored so a mirror coat does not collapse
    // the highlight to a single sub-pixel point.
    float a = max(material_params(material).y * material_params(material).y, 0.002);

    float NL = saturate(dot(N, L));
    float NV = max(dot(N, V), 1e-4);

    if (NL <= 0.0)
        return 0;

    float3 H = normalize(V + L);
    float NH = saturate(dot(N, H));
    float VH = saturate(dot(V, H));

    float F = COAT_F0 + (1.0 - COAT_F0) * pow(1.0 - VH, 5.0);
    float spec = coat_D_ggx(NH, a) * coat_V_smith(NL, NV, a) * F;

    return shadow * weight * spec * NL * 3.14159265;
}

#endif
