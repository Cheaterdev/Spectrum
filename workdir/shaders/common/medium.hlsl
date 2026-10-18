#ifndef MEDIUM_HLSL
#define MEDIUM_HLSL

// Absorption of a transmissive solid, as MaterialX's standard_surface and
// OpenPBR describe it: `tint` is the colour light has after travelling `depth`
// through the medium. A depth of zero (the files' default) means no volume at all:
// the tint is applied once, at the interface, instead.

static const float MEDIUM_MIN_DEPTH = 1e-4;

// Beer-Lambert coefficient per unit distance.
float3 medium_sigma(float3 tint, float depth)
{
    return depth > MEDIUM_MIN_DEPTH ? -log(max(tint, 0.001)) / depth : float3(0, 0, 0);
}

// What the interface itself multiplies the transmitted light by.
float3 medium_entry_tint(float3 tint, float depth)
{
    return depth > MEDIUM_MIN_DEPTH ? float3(1, 1, 1) : tint;
}

#endif
