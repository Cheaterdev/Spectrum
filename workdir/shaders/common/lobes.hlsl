#ifndef LOBES_HLSL
#define LOBES_HLSL

#include "coat.hlsl"
#include "sheen.hlsl"
#include "aniso.hlsl"

// Every extra specular lobe a material's shading model adds to the direct sun
// term, behind one call so the lighting passes do not each list them.
//
// F0 and the perceptual roughness are the base layer's (G-buffer albedo/metallic
// and the normals alpha); only the anisotropic model reads them, since it is the
// one that replaces the base specular instead of adding a layer above it.
float3 material_sun_lobes(float4 material, float3 N, float3 V, float3 L, float shadow, float3 F0, float roughness)
{
    return coat_sun_specular(material, N, V, L, shadow)
         + sheen_sun_specular(material, N, V, L, shadow)
         + aniso_sun_specular(material, N, V, L, shadow, F0, roughness);
}

#endif
