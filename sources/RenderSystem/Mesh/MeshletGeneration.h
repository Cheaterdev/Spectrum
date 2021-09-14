

void Meshletize(
    UINT maxVerts, UINT maxPrims,
    const UINT* indices, UINT indexCount,
    const float3* positions, UINT vertexCount,
    std::vector<InlineMeshlet<UINT>>& output
);


void BuildAdjacencyList(
    const uint32_t* indices, uint32_t indexCount,
    const float3* positions, uint32_t vertexCount,
    uint32_t* adjacency
);

float4 MinimumBoundingSphere(float3* points, uint32_t count);


HRESULT ComputeCullData(
    const float3* positions, uint32_t vertexCount,
    InlineMeshlet<UINT>& meshlet,
    DWORD flags
);