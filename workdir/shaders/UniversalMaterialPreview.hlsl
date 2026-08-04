// Per-node preview capture harness for the material graph editor.
//
// Concatenated the same way as UniversalMaterial.hlsl: MaterialContext
// builds this material's inline compute source as
//   uniforms (MaterialCB) + this file + the generated graph body
// The generated body is the *same* per-node HLSL your production pixel
// shader gets (see MaterialContext::start / MaterialTNode<T>::operator() in
// Materials/Values.ixx) plus one extra write per node injected right after
// each node's value is computed, so a single dispatch fills every node's
// preview slice at once. `pos`/`tc` below are what that generated body
// expects to find in scope (named exactly as MaterialGraph::start wires
// them for the pixel path).
#include "autogen/MaterialPreviewInfo.h"

#define Sampler linearSampler

// The generated per-node capture writes (see MaterialContext::capture_value)
// land inside COMPILED_FUNC's own body, not inside CS() -- a separate
// function, so CS()'s SV_DispatchThreadID parameter isn't in scope there.
// Global, set once at the top of CS(), same idea as GetMaterialPreviewInfo()
// being a global accessor rather than a threaded-through parameter.
static uint3 preview_tid;

Texture2D get_texture(uint i)
{
    return GetMaterialPreviewInfo().GetTextures(i);
}

void spec_to_metallic(float4 albedo, float3 specular, out float4 mat_albedo, out float metallic)
{
    float3 metal_rgb = (float3(1, 1, 1) / (albedo.rgb / specular + 1));
    metallic = (metal_rgb.x + metal_rgb.y + metal_rgb.z) / 3;

    mat_albedo.rgb = specular / metal_rgb;
    mat_albedo.w = albedo.w;
}

// Tiled textures aren't previewable yet (TiledTextureNode::operator() is a
// no-op in the graph today) -- kept only so a graph containing one still
// compiles instead of failing the preview build.
float4 tile_sample(Texture2D<float4> tex, SamplerState s, float2 tc, Texture2D<float4> residency, RWByteAddressBuffer visibility)
{
    return float4(1, 0, 0, 0);
}

#define sample(tex, s, tc, lod) get_texture(tex).SampleLevel(s, tc, lod);

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f, out float4 g, out float h, out float ior, float lod);

[numthreads(8, 8, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    uint w, hh, elements;
    GetMaterialPreviewInfo().GetResults().GetDimensions(w, hh, elements);

    if (id.x >= w || id.y >= hh)
        return;

    preview_tid = id;

    float2 tc  = (float2(id.xy) + 0.5) / float2(w, hh);
    float3 pos = float3(0, 0, 0);

    float4 color      = 1;
    float  metallic    = 1;
    float  roughness   = 1;
    float4 normal      = 0;
    float4 glow        = 0;
    float  opacity     = 1;
    float  refraction  = 1;

    COMPILED_FUNC(pos, tc, color, metallic, roughness, normal, glow, opacity, refraction, 0);
}
