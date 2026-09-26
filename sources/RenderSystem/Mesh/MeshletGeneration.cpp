#include <Core_defs.h>
import Core;
#include "MeshletGeneration.h"

import meshoptimizer;
import HAL;
import Graphics;

static_assert(sizeof(float3) == 3 * sizeof(float));

static Table::Meshes::MeshletCullData pack_cull_data(const meshopt_Bounds& b)
{
    Table::Meshes::MeshletCullData c;
    c.BoundingSphere = float4(b.center[0], b.center[1], b.center[2], b.radius);

    // The s8 axis/cutoff, not the float ones: meshopt pads the s8 cutoff for
    // the axis quantization error, which the shader's decoded test relies on
    // to stay conservative.
    uint8_t* cone = reinterpret_cast<uint8_t*>(&c.NormalCone);
    cone[0] = static_cast<uint8_t>(b.cone_axis_s8[0]);
    cone[1] = static_cast<uint8_t>(b.cone_axis_s8[1]);
    cone[2] = static_cast<uint8_t>(b.cone_axis_s8[2]);
    cone[3] = static_cast<uint8_t>(b.cone_cutoff_s8);
    return c;
}

void BuildMeshlets(
    const uint* indices, uint index_count,
    const float3* positions, uint vertex_count,
    std::vector<InlineMeshlet<uint>>& output
)
{
    output.clear();
    if (index_count == 0)
        return;

    const float* position_data = reinterpret_cast<const float*>(positions);

    size_t max_meshlets = meshopt_buildMeshletsBound(index_count, MESHLET_MAX_VERTICES, MESHLET_MAX_TRIANGLES);
    std::vector<meshopt_Meshlet> meshlets(max_meshlets);
    std::vector<unsigned int> meshlet_vertices(max_meshlets * MESHLET_MAX_VERTICES);
    std::vector<unsigned char> meshlet_triangles(max_meshlets * MESHLET_MAX_TRIANGLES * 3);

    const float cone_weight = 0.25f;
    size_t count = meshopt_buildMeshlets(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(),
        indices, index_count, position_data, vertex_count, sizeof(float3),
        MESHLET_MAX_VERTICES, MESHLET_MAX_TRIANGLES, cone_weight);

    output.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        const meshopt_Meshlet& m = meshlets[i];
        unsigned int* verts = &meshlet_vertices[m.vertex_offset];
        unsigned char* tris = &meshlet_triangles[m.triangle_offset];

        meshopt_optimizeMeshlet(verts, tris, m.triangle_count, m.vertex_count);
        meshopt_Bounds bounds = meshopt_computeMeshletBounds(verts, tris, m.triangle_count,
            position_data, vertex_count, sizeof(float3));

        auto& out = output.emplace_back();
        out.primitive_offset = 0;
        out.unique_offset = 0;
        out.UniqueVertexIndices.assign(verts, verts + m.vertex_count);
        out.PrimitiveIndices.assign(tris, tris + m.triangle_count * 3);
        out.cull_data = pack_cull_data(bounds);
    }
}
