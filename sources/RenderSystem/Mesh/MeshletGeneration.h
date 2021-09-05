

template <typename T>
struct InlineMeshlet
{
    struct PackedTriangle
    {
        UINT i0 : 10;
        UINT i1 : 10;
        UINT i2 : 10;
        UINT spare : 2;
    };

    std::vector<T>              UniqueVertexIndices;
    std::vector<PackedTriangle> PrimitiveIndices;
};

void Meshletize(
    UINT maxVerts, UINT maxPrims,
    const UINT* indices, UINT indexCount,
    const DirectX::XMFLOAT3* positions, UINT vertexCount,
    std::vector<InlineMeshlet<UINT>>& output
);