#ifndef SHADING_MODEL_HLSL
#define SHADING_MODEL_HLSL

// GBuffer_Material (RGBA8_UNORM): alpha is the shading model, rgb are up to
// three parameters whose meaning depends on it. Cleared to 0 = STANDARD, so
// anything that doesn't write the target (sky, decals, debug draws) lights as a
// plain metallic-roughness surface.
//
//   STANDARD   -- rgb unused
//   CLEARCOAT  -- r: coat weight, g: coat roughness, b: unused
//   SHEEN      -- r: sheen weight, g: sheen roughness, b: tint (0 white, 1 like the base colour)
//   ANISOTROPIC-- r: anisotropy, g: tangent angle / pi in the normal's own frame
//                 (see material_frame / aniso_encode_tangent), b: unused
//   SUBSURFACE -- rgb: scatter radius per channel, sqrt-encoded (sss_encode_radius)
//   THINFILM   -- r: film thickness / THINFILM_MAX_NM, g: (film index - 1) / 2
//
// A material has one model, not a combination: that is what keeps this at four
// bytes per pixel.

#define SHADING_MODEL_STANDARD  0
#define SHADING_MODEL_CLEARCOAT 1
#define SHADING_MODEL_SHEEN     2
#define SHADING_MODEL_ANISOTROPIC 3
#define SHADING_MODEL_SUBSURFACE  4
#define SHADING_MODEL_THINFILM    5

// Film thickness is stored in nanometres / this, so 8 bits give ~4 nm steps -- fine
// enough for the colour shifts, which change over tens of nm.
#define THINFILM_MAX_NM 1000.0

// Scatter radius is stored as sqrt(radius / SSS_MAX_RADIUS): 8 bits linear would
// resolve nothing below 1/255 of the maximum, and skin-like radii are small.
#define SSS_MAX_RADIUS 4.0

float4 pack_material(uint model, float3 params)
{
    return float4(saturate(params), model / 255.0);
}

uint material_model(float4 packed)
{
    return (uint)round(packed.a * 255.0);
}

float3 material_params(float4 packed)
{
    return packed.rgb;
}

// An orthonormal frame around N that depends on nothing but N, so a pass that
// only has the G-buffer normal rebuilds the exact frame the material pass used
// (Duff et al., "Building an Orthonormal Basis, Revisited"). Continuous except
// across N.z = 0, where it jumps; the stored tangent angle is relative to it.
void material_frame(float3 N, out float3 U, out float3 B)
{
    float s = N.z >= 0.0 ? 1.0 : -1.0;
    float a = -1.0 / (s + N.z);
    float b = N.x * N.y * a;
    U = float3(1.0 + s * N.x * N.x * a, s * b, -s * N.x);
    B = float3(b, s + N.y * N.y * a, -N.y);
}

// Tangent -> G-buffer value in [0,1): the angle of T in the frame above, folded
// to [0, pi) because a tangent and its negation describe the same highlight.
float aniso_encode_tangent(float3 N, float3 T)
{
    float3 U, B;
    material_frame(N, U, B);

    float phi = atan2(dot(T, B), dot(T, U));

    if (phi < 0.0)
        phi += 3.14159265;

    return saturate(phi / 3.14159265);
}

float3 aniso_decode_tangent(float3 N, float encoded)
{
    float3 U, B;
    material_frame(N, U, B);

    float phi = encoded * 3.14159265;
    return cos(phi) * U + sin(phi) * B;
}

float3 sss_encode_radius(float3 radius)
{
    return sqrt(saturate(radius / SSS_MAX_RADIUS));
}

float3 sss_decode_radius(float3 encoded)
{
    return encoded * encoded * SSS_MAX_RADIUS;
}

float thinfilm_thickness(float4 packed)
{
    return packed.r * THINFILM_MAX_NM;
}

float thinfilm_ior(float4 packed)
{
    return 1.0 + packed.g * 2.0;
}

// What the material graph's extra outputs select. A lobe that is effectively
// off stays STANDARD so lighting skips it. One model per material, in priority
// order: coat, sheen, anisotropy, subsurface, thin film. `rotation` is the tangent's turn
// about N in turns; N and T are the shading normal and the mesh tangent; the
// subsurface radius is weight * scale * radius, in scene units.
float4 pack_material_from_graph(float coat, float coat_roughness, float sheen, float sheen_roughness,
                                float anisotropy, float rotation,
                                float sss_weight, float3 sss_radius, float sss_scale, float sheen_tint,
                                float film_thickness, float film_ior,
                                float3 N, float3 T)
{
    if (coat > 0.001)
        return pack_material(SHADING_MODEL_CLEARCOAT, float3(coat, coat_roughness, 0.0));

    if (sheen > 0.001)
        return pack_material(SHADING_MODEL_SHEEN, float3(sheen, sheen_roughness, sheen_tint));

    if (anisotropy > 0.001)
    {
        T = normalize(T - N * dot(N, T));
        float theta = rotation * 6.28318531;
        T = T * cos(theta) + cross(N, T) * sin(theta);
        return pack_material(SHADING_MODEL_ANISOTROPIC, float3(anisotropy, aniso_encode_tangent(N, T), 0.0));
    }

    if (sss_weight > 0.001)
        return pack_material(SHADING_MODEL_SUBSURFACE, sss_encode_radius(sss_radius * (sss_scale * sss_weight)));

    if (film_thickness > 1.0)
        return pack_material(SHADING_MODEL_THINFILM, float3(film_thickness / THINFILM_MAX_NM, (film_ior - 1.0) / 2.0, 0.0));

    return pack_material(SHADING_MODEL_STANDARD, 0.0);
}

// Without a thin film.
float4 pack_material_from_graph(float coat, float coat_roughness, float sheen, float sheen_roughness,
                                float anisotropy, float rotation,
                                float sss_weight, float3 sss_radius, float sss_scale, float sheen_tint,
                                float3 N, float3 T)
{
    return pack_material_from_graph(coat, coat_roughness, sheen, sheen_roughness, anisotropy, rotation, sss_weight, sss_radius, sss_scale, sheen_tint, 0.0, 1.5, N, T);
}

// Without a sheen tint.
float4 pack_material_from_graph(float coat, float coat_roughness, float sheen, float sheen_roughness,
                                float anisotropy, float rotation,
                                float sss_weight, float3 sss_radius, float sss_scale,
                                float3 N, float3 T)
{
    return pack_material_from_graph(coat, coat_roughness, sheen, sheen_roughness, anisotropy, rotation, sss_weight, sss_radius, sss_scale, 0.0, N, T);
}

// Without subsurface -- for callers (and tests) that only deal in the other lobes.
float4 pack_material_from_graph(float coat, float coat_roughness, float sheen, float sheen_roughness,
                                float anisotropy, float rotation, float3 N, float3 T)
{
    return pack_material_from_graph(coat, coat_roughness, sheen, sheen_roughness, anisotropy, rotation, 0.0, 1.0, 1.0, N, T);
}

#endif
