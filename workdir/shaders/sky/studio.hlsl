// A photo studio as an environment, for the material preview: a dark neutral cyclorama
// and a handful of softboxes. Glass, metal and coats show their material in what they
// reflect and refract, and a plain sky gives them almost nothing to show it with.
//
// The key softbox sits along the sun direction, so its reflection lines up with the sun's
// own highlight; the other lights are placed relative to it. `fill` scales every light but
// the key. Radiance is linear and unbounded (the cube is R11G11B10).

// A rectangular light facing `centre` (unit), `half_extent` in tangent-plane units
// (tan of the half angle), soft over `feather`.
float studio_box(float3 v, float3 centre, float2 half_extent, float feather)
{
    float d = dot(v, centre);
    if (d <= 0.0)
        return 0.0;

    float3 up = abs(centre.y) > 0.95 ? float3(1, 0, 0) : float3(0, 1, 0);
    float3 right = normalize(cross(up, centre));
    float3 up2 = cross(centre, right);

    float2 p = float2(dot(v, right), dot(v, up2)) / d;
    float2 inside = saturate((half_extent - abs(p)) / feather);

    return smoothstep(0.0, 1.0, inside.x) * smoothstep(0.0, 1.0, inside.y);
}

float3 studio_direction(float azimuth, float elevation)
{
    return float3(cos(elevation) * cos(azimuth), sin(elevation), cos(elevation) * sin(azimuth));
}

float3 studio_environment(float3 v, float3 sun, float fill)
{
    v = normalize(v);

    // Backdrop: darker at the floor, a little brighter overhead.
    float3 colour = lerp(float3(0.025, 0.026, 0.03), float3(0.11, 0.115, 0.125), smoothstep(-0.3, 0.9, v.y));

    float sun_azimuth = atan2(sun.z, sun.x);

    // Key: large and warm, along the sun.
    colour += studio_box(v, normalize(sun), float2(0.55, 0.45), 0.3) * float3(1.0, 0.96, 0.9) * 14.0;

    // Fill: opposite the key, wide and cool.
    colour += studio_box(v, studio_direction(sun_azimuth + 2.6, 0.35), float2(0.9, 0.6), 0.5) * float3(0.85, 0.92, 1.0) * 3.0 * fill;

    // Two tall strips behind the object: a bright edge on every curved surface.
    colour += studio_box(v, studio_direction(sun_azimuth + 1.9, 0.15), float2(0.1, 0.9), 0.08) * 7.0 * fill;
    colour += studio_box(v, studio_direction(sun_azimuth - 1.9, 0.15), float2(0.1, 0.9), 0.08) * 7.0 * fill;

    // Overhead panel.
    colour += studio_box(v, float3(0, 1, 0), float2(0.9, 0.9), 0.5) * 2.2 * fill;

    return colour;
}
