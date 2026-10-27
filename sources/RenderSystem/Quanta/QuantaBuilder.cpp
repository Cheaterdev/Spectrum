#include "clusterlod.h"
import Core;
#include "QuantaBuilder.h"

import meshoptimizer;
import HAL;
import Graphics;

// Same clusterlod setup as MeshletGeneration.cpp, kept separate on purpose: Quanta is its own system and
// MeshAsset's meshlet path must not change while it is built.

static float4 to_sphere(const clodBounds& b)
{
    return float4(b.center[0], b.center[1], b.center[2], b.radius);
}

static uint32_t pack_cone(const meshopt_Bounds& b)
{
    // The s8 axis/cutoff, not the float ones: meshopt pads the s8 cutoff for the axis quantization error,
    // which a decoded cone test relies on to stay conservative.
    uint32_t cone = 0;
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&cone);
    bytes[0] = static_cast<uint8_t>(b.cone_axis_s8[0]);
    bytes[1] = static_cast<uint8_t>(b.cone_axis_s8[1]);
    bytes[2] = static_cast<uint8_t>(b.cone_axis_s8[2]);
    bytes[3] = static_cast<uint8_t>(b.cone_cutoff_s8);
    return cone;
}

Quanta::Mesh BuildQuanta(
    const uint32_t* indices, uint32_t index_count,
    const Quanta::Vertex* vertices, uint32_t vertex_count
)
{
    using Quanta::Vertex;

    if (index_count == 0)
        return {};

    const float* positions = reinterpret_cast<const float*>(&vertices[0].position);

    clodConfig config = clodDefaultConfig(Quanta::MaxClusterTriangles);
    config.max_vertices = Quanta::MaxClusterVertices;
    config.simplify_fallback_sloppy = false;
    config.partition_sort = true;

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

    // Lock the mesh's open border: clusterlod only locks borders between its own groups, so on flat
    // geometry the simplifier otherwise pulls the outline inward at zero reported error. Edges are matched
    // by position so UV/normal seams don't count as borders.
    std::vector<unsigned char> border_lock(vertex_count, 0);
    {
        std::vector<unsigned int> remap(vertex_count);
        meshopt_generatePositionRemap(remap.data(), positions, vertex_count, sizeof(Vertex));

        std::unordered_map<uint64_t, uint32_t> edge_uses;
        edge_uses.reserve(index_count);
        for (uint32_t t = 0; t + 2 < index_count; t += 3)
            for (uint32_t e = 0; e < 3; e++)
            {
                uint32_t a = remap[indices[t + e]], b = remap[indices[t + (e + 1) % 3]];
                if (a == b)
                    continue;
                edge_uses[(uint64_t(std::min(a, b)) << 32) | std::max(a, b)]++;
            }

        std::vector<unsigned char> border_position(vertex_count, 0);
        for (auto& [key, uses] : edge_uses)
            if (uses == 1)
            {
                border_position[uint32_t(key >> 32)] = 1;
                border_position[uint32_t(key & 0xffffffff)] = 1;
            }

        for (uint32_t v = 0; v < vertex_count; v++)
            if (border_position[remap[v]])
                border_lock[v] = meshopt_SimplifyVertex_Lock;
    }
    mesh.vertex_lock = border_lock.data();

    std::vector<Quanta::GroupInput> groups;
    std::vector<clodBounds> clod_groups;
    std::vector<uint32_t> clod_families;   // per clusterlod group: the packed family of its Quanta groups

    std::vector<unsigned int> local_vertices(Quanta::MaxClusterVertices);
    std::vector<unsigned char> local_triangles(Quanta::MaxClusterTriangles * 3);
    std::vector<float4> refined_spheres;

    clodBuild(config, mesh, [&](clodGroup group, const clodCluster* clusters, size_t cluster_count) -> int
    {
        // Groups of at most GroupMaxClusters, sharing this group's LOD data.
        const uint32_t family_first = static_cast<uint32_t>(groups.size());
        const uint32_t family_count = static_cast<uint32_t>((cluster_count + Quanta::GroupMaxClusters - 1) / Quanta::GroupMaxClusters);
        ASSERT(family_count >= 1 && family_count <= 256 && family_first < (1u << Quanta::FamilyCountShift));
        const uint32_t family = Quanta::pack_family(family_first, family_count);

        for (size_t chunk = 0; chunk < cluster_count; chunk += Quanta::GroupMaxClusters)
        {
            size_t chunk_count = std::min<size_t>(Quanta::GroupMaxClusters, cluster_count - chunk);

            Quanta::GroupInput out;
            out.lod_sphere = to_sphere(group.simplified);
            out.lod_error = group.simplified.error;
            out.depth = static_cast<uint32_t>(group.depth);
            out.family = family;

            bool has_original = false;
            float min_refined = std::numeric_limits<float>::max();
            refined_spheres.clear();

            for (size_t i = chunk; i < chunk + chunk_count; i++)
            {
                const clodCluster& cluster = clusters[i];
                size_t triangle_count = cluster.index_count / 3;

                size_t unique = clodLocalIndices(local_vertices.data(), local_triangles.data(), cluster.indices, cluster.index_count);
                ASSERT(unique <= Quanta::MaxClusterVertices && triangle_count <= Quanta::MaxClusterTriangles);

                meshopt_Bounds bounds = meshopt_computeMeshletBounds(local_vertices.data(), local_triangles.data(), triangle_count,
                    positions, vertex_count, sizeof(Vertex));

                Quanta::ClusterInput c;
                c.unique_vertices.assign(local_vertices.begin(), local_vertices.begin() + unique);
                c.triangles.assign(local_triangles.begin(), local_triangles.begin() + triangle_count * 3);
                c.bounding_sphere = float4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius);
                c.normal_cone = pack_cone(bounds);
                c.lod_level = static_cast<uint32_t>(group.depth);
                if (cluster.refined >= 0)
                {
                    c.refined_error = clod_groups[cluster.refined].error;
                    c.refined_sphere = to_sphere(clod_groups[cluster.refined]);
                    c.refined_family = clod_families[cluster.refined];
                    min_refined = std::min(min_refined, c.refined_error);
                    refined_spheres.push_back(c.refined_sphere);
                }
                else
                {
                    c.refined_error = -1;
                    c.refined_sphere = float4(0, 0, 0, 0);
                    has_original = true;
                }

                out.clusters.push_back(std::move(c));
            }

            out.refined_error = has_original ? -1.0f : min_refined;
            out.refined_sphere = float4(0, 0, 0, 0);
            if (!has_original)
            {
                out.refined_sphere = refined_spheres[0];
                for (size_t i = 1; i < refined_spheres.size(); i++)
                    out.refined_sphere = Quanta::merge_spheres(out.refined_sphere, refined_spheres[i]);
            }
            groups.push_back(std::move(out));
        }

        clod_groups.push_back(group.simplified);
        clod_families.push_back(family);
        return static_cast<int>(clod_groups.size() - 1);
    });

    return Quanta::Pack(std::move(groups), vertices, index_count / 3);
}
