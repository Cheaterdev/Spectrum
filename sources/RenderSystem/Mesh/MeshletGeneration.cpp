#include <Core_defs.h>
#include "clusterlod.h"
import Core;
#include "MeshletGeneration.h"

import meshoptimizer;
import HAL;
import Graphics;

using Vertex = Table::Meshes::mesh_vertex_input::Compiled;

// clodBuild reads positions and (normal, tc) straight out of the vertex array:
// pos/normal/tc/tangent with no padding.
static_assert(sizeof(float3) == 3 * sizeof(float));
static_assert(sizeof(Vertex) == 12 * sizeof(float));

static float4 to_sphere(const clodBounds& b)
{
    return float4(b.center[0], b.center[1], b.center[2], b.radius);
}

static void pack_cone(Table::Meshes::MeshletCullData& c, const meshopt_Bounds& b)
{
    c.BoundingSphere = float4(b.center[0], b.center[1], b.center[2], b.radius);

    // The s8 axis/cutoff, not the float ones: meshopt pads the s8 cutoff for
    // the axis quantization error, which the shader's decoded test relies on
    // to stay conservative.
    uint8_t* cone = reinterpret_cast<uint8_t*>(&c.NormalCone);
    cone[0] = static_cast<uint8_t>(b.cone_axis_s8[0]);
    cone[1] = static_cast<uint8_t>(b.cone_axis_s8[1]);
    cone[2] = static_cast<uint8_t>(b.cone_axis_s8[2]);
    cone[3] = static_cast<uint8_t>(b.cone_cutoff_s8);
}

static float4 enclosing_sphere(const std::vector<float4>& spheres)
{
    static_assert(sizeof(float4) == 4 * sizeof(float));
    meshopt_Bounds b = meshopt_computeSphereBounds(&spheres[0].x, spheres.size(), sizeof(float4), &spheres[0].w, sizeof(float4));
    return float4(b.center[0], b.center[1], b.center[2], b.radius);
}

uint BuildMeshlets(
    const uint* indices, uint index_count,
    const Vertex* vertices, uint vertex_count,
    std::vector<InlineMeshlet<uint>>& output,
    std::vector<Table::Meshes::MeshletGroup>& groups,
    std::vector<Table::Meshes::MeshletGroup>& nodes
)
{
    output.clear();
    groups.clear();
    nodes.clear();
    if (index_count == 0)
        return 0;

    const float* positions = reinterpret_cast<const float*>(&vertices[0].pos);

    clodConfig config = clodDefaultConfig(MESHLET_MAX_TRIANGLES);
    // The default ties max_vertices to max_triangles; the mesh shaders output 64.
    config.max_vertices = MESHLET_MAX_VERTICES;
    // No sloppy fallback: its vertex clustering ignores topology, and on flat
    // geometry its error still comes out ~0, so the damaged level (floors that
    // grew or lost area) won at every threshold. A group that regular
    // simplification can't reduce stays terminal instead.
    config.simplify_fallback_sloppy = false;
    // Spatially ordered groups, so the 32 consecutive ones a hierarchy node
    // covers are neighbours and its bounds stay tight.
    config.partition_sort = true;

    // Normals and UVs guide the simplifier; UV seams are protected (attributes
    // 3, 4) since permissive mode would otherwise collapse across them.
    const float attribute_weights[5] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

    clodMesh mesh = {};
    mesh.indices = indices;
    mesh.index_count = index_count;
    mesh.vertex_count = vertex_count;
    mesh.vertex_positions = positions;
    mesh.vertex_positions_stride = sizeof(Vertex);
    mesh.vertex_attributes = reinterpret_cast<const float*>(&vertices[0].normal);
    mesh.vertex_attributes_stride = sizeof(Vertex);
    mesh.attribute_weights = attribute_weights;
    mesh.attribute_count = 5;
    mesh.attribute_protect_mask = (1 << 3) | (1 << 4);

    // Lock the mesh's open border. clusterlod only locks borders between its own
    // groups, so a mesh that fits in one group had a free outline: on flat
    // geometry the simplifier pulled it inward at zero reported error, and the
    // zero error made that level win at every threshold -- a permanent hole.
    // Locking also keeps separate modular meshes (floor tiles, walls) meeting.
    // Edges are matched by position so UV/normal seams don't count as borders.
    std::vector<unsigned char> border_lock(vertex_count, 0);
    {
        std::vector<unsigned int> remap(vertex_count);
        meshopt_generatePositionRemap(remap.data(), positions, vertex_count, sizeof(Vertex));

        std::unordered_map<uint64_t, uint> edge_uses;
        edge_uses.reserve(index_count);
        for (uint t = 0; t + 2 < index_count; t += 3)
            for (uint e = 0; e < 3; e++)
            {
                uint a = remap[indices[t + e]], b = remap[indices[t + (e + 1) % 3]];
                if (a == b)
                    continue;
                edge_uses[(uint64_t(std::min(a, b)) << 32) | std::max(a, b)]++;
            }

        std::vector<unsigned char> border_position(vertex_count, 0);
        for (auto& [key, uses] : edge_uses)
            if (uses == 1)
            {
                border_position[uint(key >> 32)] = 1;
                border_position[uint(key & 0xffffffff)] = 1;
            }

        for (uint v = 0; v < vertex_count; v++)
            if (border_position[remap[v]])
                border_lock[v] = meshopt_SimplifyVertex_Lock;
    }
    mesh.vertex_lock = border_lock.data();


    std::vector<clodBounds> clod_groups;

    std::vector<unsigned int> local_vertices(MESHLET_MAX_VERTICES);
    std::vector<unsigned char> local_triangles(MESHLET_MAX_TRIANGLES * 3);
    std::vector<float4> refined_spheres;

    clodBuild(config, mesh, [&](clodGroup group, const clodCluster* clusters, size_t cluster_count) -> int
    {
        // Split into records of at most MESHLET_LOD_GROUP_CLUSTERS, which bounds
        // one AS group's output (see MeshletGroup). The chunks share the LOD data.
        for (size_t chunk = 0; chunk < cluster_count; chunk += MESHLET_LOD_GROUP_CLUSTERS)
        {
            size_t chunk_count = std::min<size_t>(MESHLET_LOD_GROUP_CLUSTERS, cluster_count - chunk);

            Table::Meshes::MeshletGroup record;
            record.LodSphere = to_sphere(group.simplified);
            record.LodError = group.simplified.error;
            record.FirstCluster = static_cast<uint>(output.size());
            record.ClusterCount = static_cast<uint>(chunk_count);

            bool has_original = false;
            float min_refined = std::numeric_limits<float>::max();
            refined_spheres.clear();

            for (size_t i = chunk; i < chunk + chunk_count; i++)
            {
                const clodCluster& cluster = clusters[i];
                size_t triangle_count = cluster.index_count / 3;

                size_t unique = clodLocalIndices(local_vertices.data(), local_triangles.data(), cluster.indices, cluster.index_count);
                ASSERT(unique <= MESHLET_MAX_VERTICES && triangle_count <= MESHLET_MAX_TRIANGLES);

                // Culling bounds from the cluster's own geometry; the clod bounds
                // are group-merged for LOD monotonicity and too loose for culling.
                meshopt_Bounds bounds = meshopt_computeMeshletBounds(local_vertices.data(), local_triangles.data(), triangle_count,
                    positions, vertex_count, sizeof(Vertex));

                InlineMeshlet<uint> out;
                out.primitive_offset = 0;
                out.unique_offset = 0;
                out.UniqueVertexIndices.assign(local_vertices.begin(), local_vertices.begin() + unique);
                out.PrimitiveIndices.assign(local_triangles.begin(), local_triangles.begin() + triangle_count * 3);

                auto& c = out.cull_data;
                pack_cone(c, bounds);
                c.LodLevel = static_cast<uint>(group.depth);
                if (cluster.refined >= 0)
                {
                    c.RefinedError = clod_groups[cluster.refined].error;
                    c.RefinedSphere = to_sphere(clod_groups[cluster.refined]);
                    min_refined = std::min(min_refined, c.RefinedError);
                    refined_spheres.push_back(c.RefinedSphere);
                }
                else
                {
                    c.RefinedError = -1;
                    c.RefinedSphere = float4(0, 0, 0, 0);
                    has_original = true;
                }

                output.emplace_back(std::move(out));
            }

            record.RefinedError = has_original ? -1.0f : min_refined;
            record.RefinedSphere = has_original ? float4(0, 0, 0, 0) : enclosing_sphere(refined_spheres);
            groups.push_back(record);
        }

        clod_groups.push_back(group.simplified);
        return static_cast<int>(clod_groups.size() - 1);
    });

    // clodBuild emits every depth-0 group before any deeper one, and depth-0
    // groups hold exactly the original clusters, so emission order already
    // puts them first -- which the passes without cluster LOD rely on.
    uint original_count = 0;
    while (original_count < output.size() && output[original_count].cull_data.RefinedError < 0)
        original_count++;
    for (size_t i = original_count; i < output.size(); i++)
        ASSERT(output[i].cull_data.RefinedError >= 0);

    std::vector<float4> spheres;
    for (size_t first = 0; first < groups.size(); first += MESHLET_LOD_NODE_GROUPS)
    {
        size_t count = std::min<size_t>(MESHLET_LOD_NODE_GROUPS, groups.size() - first);

        Table::Meshes::MeshletGroup node;
        node.FirstCluster = static_cast<uint>(first);
        node.ClusterCount = static_cast<uint>(count);

        spheres.clear();
        node.LodError = 0;
        for (size_t g = first; g < first + count; g++)
        {
            spheres.push_back(groups[g].LodSphere);
            node.LodError = std::max(node.LodError, groups[g].LodError);
        }
        node.LodSphere = enclosing_sphere(spheres);

        spheres.clear();
        node.RefinedError = std::numeric_limits<float>::max();
        for (size_t g = first; g < first + count; g++)
        {
            node.RefinedError = std::min(node.RefinedError, groups[g].RefinedError);
            spheres.push_back(groups[g].RefinedSphere);
        }
        node.RefinedSphere = node.RefinedError < 0 ? float4(0, 0, 0, 0) : enclosing_sphere(spheres);

        nodes.push_back(node);
    }

    return original_count;
}
