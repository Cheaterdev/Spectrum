// Leading fields of gbuffer/mesh_shader.hlsl's vertex_output: the PS reads only
// this prefix of what the mesh shader writes.
struct vertex_output
{
    float4 pos : SV_POSITION;
    float3 wpos : POSITION;
    float3 normal : NORMAL;
};

#include "../common/common.hlsl"

#ifdef BUILD_FUNC_PS
#include "../autogen/FrameInfo.h"
#include "../autogen/MeshInfo.h"
#include "../autogen/DebugViewTint.h"
#include "../autogen/DebugViewFrustum.h"

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

float3 id_color(uint id)
{
    uint h = id * 2654435761u;
    h ^= h >> 15;
    h *= 2246822519u;
    h ^= h >> 13;
    return 0.35 + 0.65 * float3((h & 0xFF) / 255.0, ((h >> 8) & 0xFF) / 255.0, ((h >> 16) & 0xFF) / 255.0);
}

float4 PS(vertex_output i) : SV_TARGET0
{
    float3 V = normalize(GetFrameInfo().GetCamera().GetPosition().xyz - i.wpos);
    float3 N = normalize(i.normal);
    float headlight = 0.25 + 0.75 * abs(dot(N, V));
    float4 tint = GetDebugViewTint().GetTint();
    float3 color = lerp(id_color(GetMeshInfo().GetObject_id()), tint.rgb, tint.a);

    DebugViewFrustum view = GetDebugViewFrustum();
    if (view.GetOutside_brightness() < 1 && !inside_frustum(view.GetFrustum(), i.wpos))
    {
        // Darkened and desaturated, so the inside reads as "lit".
        float luma = dot(color, float3(0.299, 0.587, 0.114));
        color = lerp(luma.xxx, color, 0.35) * view.GetOutside_brightness();
    }

    return float4(color * headlight, 1);
}
#endif
