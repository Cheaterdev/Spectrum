// Same layout as gbuffer/mesh_shader.hlsl's outputs. Declared in full, not just
// the fields used: the per-primitive attributes come after the per-vertex ones
// in the signature, so a vertex-prefix-only PS input wouldn't line up with them.
struct vertex_output
{
    float4 pos : SV_POSITION;
    float3 wpos : POSITION;
    float3 normal : NORMAL;
    float3 binormal : BINORMAL;
    float3 tangent : TANGENT;
    float2 tc : TEXCOORD;
    float4 cur_pos : CUR_POSITION;

    float4 prev_pos : PREV_POSITION;
    float dist : DISTANCE;
};

struct primitive_output
{
    uint meshlet : MESHLET_ID;
    uint status : MESHLET_STATUS;
};

#include "../common/common.hlsl"

#ifdef BUILD_FUNC_PS
#include "../autogen/FrameInfo.h"
#include "../autogen/MeshInfo.h"
#include "../autogen/DebugViewDraw.h"
#include "../autogen/DebugViewFrustum.h"

// Must match gbuffer/mesh_shader.hlsl's MESHLET_* codes.
static const uint MESHLET_DRAWN = 3;

// Tints for meshlets the main view culled inside a part it drew, by reason:
// frustum, backface cone, Hi-Z occlusion.
static const float4 meshlet_culled_tints[3] =
{
    float4(1.0, 0.25, 0.85, 0.85),
    float4(0.3, 0.45, 1.0, 0.85),
    float4(1.0, 0.9, 0.15, 0.85),
};

uint hash(uint h)
{
    h *= 2654435761u;
    h ^= h >> 15;
    h *= 2246822519u;
    h ^= h >> 13;
    return h;
}

float3 id_color(uint id)
{
    uint h = hash(id);
    return 0.35 + 0.65 * float3((h & 0xFF) / 255.0, ((h >> 8) & 0xFF) / 255.0, ((h >> 16) & 0xFF) / 255.0);
}

// Same test as gather_pipeline.hlsl's intersect(): inside when on the positive
// side of all six planes.
bool inside_frustum(Frustum f, float3 p)
{
    [unroll]
    for (int i = 0; i < 6; i++)
        if (dot(p, f.GetPlanes(i).xyz) + f.GetPlanes(i).w < 0)
            return false;
    return true;
}

float4 PS(vertex_output i, primitive_output prim) : SV_TARGET0
{
    DebugViewDraw draw = GetDebugViewDraw();

    uint object_id = GetMeshInfo().GetObject_id();
    float3 color = draw.GetColor_by_meshlet() ? id_color(hash(object_id) ^ prim.meshlet) : id_color(object_id);

    float4 tint = prim.status < MESHLET_DRAWN ? meshlet_culled_tints[prim.status] : draw.GetTint();
    color = lerp(color, tint.rgb, tint.a);

    DebugViewFrustum view = GetDebugViewFrustum();
    if (view.GetOutside_brightness() < 1 && !inside_frustum(view.GetFrustum(), i.wpos))
    {
        // Darkened and desaturated, so the inside reads as "lit".
        float luma = dot(color, float3(0.299, 0.587, 0.114));
        color = lerp(luma.xxx, color, 0.35) * view.GetOutside_brightness();
    }

    float3 V = normalize(GetFrameInfo().GetCamera().GetPosition().xyz - i.wpos);
    float3 N = normalize(i.normal);
    float headlight = 0.25 + 0.75 * abs(dot(N, V));
    return float4(color * headlight, 1);
}
#endif
