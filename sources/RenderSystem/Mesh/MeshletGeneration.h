#pragma once
import Graphics;
import Core;
import HAL;

// Must match the mesh shaders' `out indices uint3 tris[...]` / `out vertices
// verts[...]` declarations. meshopt requires the triangle limit to be a
// multiple of 4.
static constexpr uint MESHLET_MAX_VERTICES = 64;
static constexpr uint MESHLET_MAX_TRIANGLES = 124;

// LOD hierarchy shape; must match gbuffer/mesh_shader.hlsl's LOD_* constants.
// One ClusterLod AS group walks one node, so their product bounds its payload.
static constexpr uint MESHLET_LOD_GROUP_CLUSTERS = 16;
static constexpr uint MESHLET_LOD_NODE_GROUPS = 32;

// Builds the cluster LOD DAG. output holds the clusters in group order, which
// puts the original full-detail ones first (their count is returned); groups
// and nodes are the two hierarchy levels (see MeshletGroup in meshrender.prism).
uint BuildMeshlets(
    const uint* indices, uint index_count,
    const Table::Meshes::mesh_vertex_input::Compiled* vertices, uint vertex_count,
    std::vector<InlineMeshlet<uint>>& output,
    std::vector<Table::Meshes::MeshletGroup>& groups,
    std::vector<Table::Meshes::MeshletGroup>& nodes
);
