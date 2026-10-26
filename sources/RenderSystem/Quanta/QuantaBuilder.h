#pragma once
import Graphics;
import Core;
import HAL;

// Builds a Quanta mesh from an indexed triangle mesh (cluster LOD through clusterlod, then Quanta::Pack).
Quanta::Mesh BuildQuanta(
    const uint32_t* indices, uint32_t index_count,
    const Quanta::Vertex* vertices, uint32_t vertex_count
);
