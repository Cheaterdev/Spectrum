// Per-node preview, 3D mode: draws the real material_tester mesh via a
// direct (non-indirect) dispatch_mesh -- see MaterialPreviewSession::dispatch
// -- instead of the flat/analytic-sphere compute dispatch in
// UniversalMaterialPreview.hlsl. Reuses mesh_shader.hlsl's generic VS/AS (the
// same ones GBufferDraw/DepthDraw/DrawAxis use -- see material_preview.sig's
// MaterialPreview3D) for real interpolated position/normal/UV, and the
// *same* capture-injected COMPILED_FUNC body (see MaterialContext::
// capture_value in Values.cpp) that the compute path uses -- only the
// harness differs (raster fragment vs. compute thread).
//
// This file is compiled as inline text (see MaterialPreviewSession::
// rebuild_pso), not as a standalone file compile -- unlike stencil.hlsl's
// PS_COLOR (a file-referenced pixel stage that gets vertex_output for free
// from however the engine's normal GraphicsPSO compile wires mesh+pixel
// files together), we have to pull mesh_shader.hlsl's vertex_output/transform
// in ourselves, hence the explicit include.
#include "mesh_shader.hlsl"

#include "autogen/MaterialPreviewInfo.h"

#define Sampler linearSampler

static uint3 preview_tid;
static float preview_shade;

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

// Tiled textures aren't previewable yet -- see UniversalMaterialPreview.hlsl.
float4 tile_sample(Texture2D<float4> tex, SamplerState s, float2 tc, Texture2D<float4> residency, RWByteAddressBuffer visibility)
{
    return float4(1, 0, 0, 0);
}

#define sample(tex, s, tc, lod) get_texture(tex).SampleLevel(s, tc, lod);

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f, out float4 g, out float h, out float ior, float lod);

// vertex_output (pos/wpos/normal/binormal/tangent/tc/...) comes from
// mesh_shader.hlsl, concatenated ahead of this file for MaterialPreview3D's
// VS/AS+PS build (same mechanism stencil.hlsl's PS_COLOR relies on).
//
// [earlydepthstencil]: this PS writes to a UAV (results[]), not a bound
// render target -- without forcing the depth test early, occluded/discarded
// fragments still execute and still write, since nothing about a UAV write
// is naturally gated by the depth test the way an OM render target write is.
// That's what was actually causing the holes/flicker -- overlapping
// front/back fragments both writing, in GPU-scheduling-dependent order,
// regardless of which one the depth test would have kept. Same fix
// stencil.hlsl's own UAV-writing PS() already uses.
[earlydepthstencil]
void PS_PREVIEW(vertex_output i)
{
    uint w, hh, elements;
    GetMaterialPreviewInfo().GetResults().GetDimensions(w, hh, elements);

    preview_tid = uint3(uint2(i.pos.xy), 0);

    if (preview_tid.x >= w || preview_tid.y >= hh)
        return;

    // Raw, unlit values -- same as 2D mode (preview_shade stays 1 there
    // too). A fake N.L light made captured values look darker/lighter than
    // what the node actually computed, which defeats the point of the
    // preview.
    preview_shade = 1;

    float4 color      = 1;
    float  metallic    = 1;
    float  roughness   = 1;
    float4 normal      = 0;
    float4 glow        = 0;
    float  opacity     = 1;
    float  refraction  = 1;

    COMPILED_FUNC(i.wpos, i.tc, color, metallic, roughness, normal, glow, opacity, refraction, 0);
}
