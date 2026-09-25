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
    return float4(id_color(GetMeshInfo().GetObject_id()) * headlight, 1);
}
#endif
