#pragma once
import Graphics;
import Core;

// Must match the mesh shaders' `out indices uint3 tris[...]` / `out vertices
// verts[...]` declarations. meshopt requires the triangle limit to be a
// multiple of 4.
static constexpr uint MESHLET_MAX_VERTICES = 64;
static constexpr uint MESHLET_MAX_TRIANGLES = 124;

void BuildMeshlets(
    const uint* indices, uint index_count,
    const float3* positions, uint vertex_count,
    std::vector<InlineMeshlet<uint>>& output
);
