#pragma once
import Graphics;
import Math;
import stl.core;
import windows;

void Meshletize(
    uint maxVerts, uint maxPrims,
    const uint* indices, uint indexCount,
    const float3* positions, uint vertexCount,
    std::vector<InlineMeshlet<uint>>& output
);


void BuildAdjacencyList(
    const uint32_t* indices, uint32_t indexCount,
    const float3* positions, uint32_t vertexCount,
    uint32_t* adjacency
);

float4 MinimumBoundingSphere(float3* points, uint32_t count);


HRESULT ComputeCullData(
    const float3* positions, uint32_t vertexCount,
    InlineMeshlet<uint>& meshlet,
    DWORD flags
);