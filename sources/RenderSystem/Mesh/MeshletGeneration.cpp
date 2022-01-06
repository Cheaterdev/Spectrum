#include "pch_render.h"
#include "MeshletGeneration.h"
#include "Assets/MeshAsset.h"
namespace internal
{
    template <typename T>
    void Meshletize(
        UINT maxVerts, UINT maxPrims,
        const T* indices, UINT indexCount,
        const vec3* positions, UINT vertexCount,
        std::vector<InlineMeshlet<T>>& output);
}



void Meshletize(
    UINT maxVerts, UINT maxPrims,
    const UINT* indices, UINT indexCount,
    const vec3* positions, UINT vertexCount,
    std::vector<InlineMeshlet<UINT>>& output
)
{
    return internal::Meshletize(maxVerts, maxPrims, indices, indexCount, positions, vertexCount, output);
}


///
// Helpers

// Sort in reverse order to use vector as a queue with pop_back.
bool CompareScores(const std::pair<UINT, float>& a, const std::pair<UINT, float>& b)
{
    return a.second > b.second;
}

vec3 ComputeNormal(vec3* tri)
{
    vec3 v01 = tri[0] - tri[1];
    vec3 v02 = tri[0] - tri[2];

    return vec3::cross(v01, v02).normalize();
}

// Compute number of triangle vertices already exist in the meshlet
template <typename T>
UINT ComputeReuse(const InlineMeshlet<T>& meshlet, T(&triIndices)[3])
{
    UINT count = 0;

    for (UINT i = 0; i < static_cast<UINT>(meshlet.UniqueVertexIndices.size()); ++i)
    {
        for (UINT j = 0; j < 3u; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == triIndices[j])
            {
                ++count;
            }
        }
    }

    return count;
}

// Computes a candidacy score based on spatial locality, orientational coherence, and vertex re-use within a meshlet.
template <typename T>
float ComputeScore(const InlineMeshlet<T>& meshlet, vec4 sphere, vec3 normal, T(&triIndices)[3], vec3* triVerts)
{
    const float reuseWeight = 0.334f;
    const float locWeight = 0.333f;
    const float oriWeight = 0.333f;

    // Vertex reuse
    UINT reuse = ComputeReuse(meshlet, triIndices);

    float reuseScore = (1.0f-float(reuse) / 3.0f);

    // Distance from center point
    float maxSq = 0;
    for (UINT i = 0; i < 3u; ++i)
    {
        vec3 v = triVerts[i] - sphere.xyz;
        maxSq = std::max(maxSq, v.length_squared());
    }
    float r = sphere.w;
    float r2 = r * r;
    float locScore = std::log(maxSq / r2 + 1);

    // Angle between normal and meshlet cone axis
    vec3 n = ComputeNormal(triVerts);
    float d = vec3::dot(n, normal);
    float oriScore = (-d + 1) / 2.0f;

    float b = reuseWeight * reuseScore + locWeight * locScore + oriWeight * oriScore;

    return b;
}

// Determines whether a candidate triangle can be added to a specific meshlet; if it can, does so.
template <typename T>
bool AddToMeshlet(UINT maxVerts, UINT maxPrims, InlineMeshlet<T>& meshlet, T(&tri)[3])
{
    // Are we already full of vertices?
    if (meshlet.UniqueVertexIndices.size() == maxVerts)
        return false;

    // Are we full, or can we store an additional primitive?
    if (meshlet.PrimitiveIndices.size() == maxPrims)
        return false;

    static const UINT Undef = UINT(-1);
    UINT indices[3] = { Undef, Undef, Undef };
    UINT newCount = 3;

    for (UINT i = 0; i < meshlet.UniqueVertexIndices.size(); ++i)
    {
        for (UINT j = 0; j < 3; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == tri[j])
            {
                indices[j] = i;
                --newCount;
            }
        }
    }

    // Will this triangle fit?
    if (meshlet.UniqueVertexIndices.size() + newCount > maxVerts)
        return false;

    // Add unique vertex indices to unique vertex index list
    for (UINT j = 0; j < 3; ++j)
    {
        if (indices[j] == Undef)
        {
            indices[j] = static_cast<UINT>(meshlet.UniqueVertexIndices.size());
            meshlet.UniqueVertexIndices.push_back(tri[j]);
        }
    }

    // Add the new primitive 
    meshlet.PrimitiveIndices.push_back(indices[0]);
    meshlet.PrimitiveIndices.push_back(indices[1]);
    meshlet.PrimitiveIndices.push_back(indices[2]);

    return true;
}

template <typename T>
bool IsMeshletFull(UINT maxVerts, UINT maxPrims, const InlineMeshlet<T>& meshlet)
{
    assert(meshlet.UniqueVertexIndices.size() <= maxVerts);
    assert(meshlet.PrimitiveIndices.size() <= maxPrims);

    return meshlet.UniqueVertexIndices.size() == maxVerts
        || meshlet.PrimitiveIndices.size() == maxPrims;
}


///
// Implementation 

template <typename T>
void internal::Meshletize(
    UINT maxVerts, UINT maxPrims,
    const T* indices, UINT indexCount,
    const vec3* positions, UINT vertexCount,
    std::vector<InlineMeshlet<T>>& output
)
{
    const UINT triCount = indexCount / 3;

    // Build a primitive adjacency list
    std::vector<UINT> adjacency;
    adjacency.resize(indexCount);

    BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency.data());

    // Rest our outputs
    output.clear();
    output.emplace_back();
    auto* curr = &output.back();

    // Bitmask of all triangles in mesh to determine whether a specific one has been added.
    std::vector<bool> checklist;
    checklist.resize(triCount);

    std::vector<vec3> m_positions;
    std::vector<vec3> normals;
    std::vector<std::pair<UINT, float>> candidates;
    std::unordered_set<UINT> candidateCheck;

    float4 psphere;
	float3 normal;

    // Arbitrarily start at triangle zero.
    UINT triIndex = 0;
    candidates.push_back(std::make_pair(triIndex, 0.0f));
    candidateCheck.insert(triIndex);

    // Continue adding triangles until 
    while (!candidates.empty())
    {
        UINT index = candidates.back().first;
        candidates.pop_back();

        T tri[3] =
        {
            indices[index * 3],
            indices[index * 3 + 1],
            indices[index * 3 + 2],
        };

        assert(tri[0] < vertexCount);
        assert(tri[1] < vertexCount);
        assert(tri[2] < vertexCount);

        // Try to add triangle to meshlet
        if (AddToMeshlet(maxVerts, maxPrims, *curr, tri))
        {
            // Success! Mark as added.
            checklist[index] = true;

            // Add m_positions & normal to list
            vec3 points[3] =
            {
                positions[tri[0]],
                positions[tri[1]],
                positions[tri[2]],
            };

            m_positions.push_back(points[0]);
            m_positions.push_back(points[1]);
            m_positions.push_back(points[2]);

            vec3 Normal= ComputeNormal(points);
            normals.push_back(Normal);

            // Compute new bounding sphere & normal axis
            psphere = MinimumBoundingSphere(m_positions.data(), static_cast<UINT>(m_positions.size()));

            float4 nsphere = MinimumBoundingSphere(normals.data(), static_cast<UINT>(normals.size()));
            normal = nsphere.xyz.normalized();

            // Find and add all applicable adjacent triangles to candidate list
            const UINT adjIndex = index * 3;

            UINT adj[3] =
            {
                adjacency[adjIndex],
                adjacency[adjIndex + 1],
                adjacency[adjIndex + 2],
            };

            for (UINT i = 0; i < 3u; ++i)
            {
                // Invalid triangle in adjacency slot
                if (adj[i] == -1)
                    continue;

                // Already processed triangle
                if (checklist[adj[i]])
                    continue;

                // Triangle already in the candidate list
                if (candidateCheck.count(adj[i]))
                    continue;

                candidates.push_back(std::make_pair(adj[i], FLT_MAX));
                candidateCheck.insert(adj[i]);
            }

            // Re-score remaining candidate triangles
            for (UINT i = 0; i < static_cast<UINT>(candidates.size()); ++i)
            {
                UINT candidate = candidates[i].first;

                T triIndices[3] =
                {
                    indices[candidate * 3],
                    indices[candidate * 3 + 1],
                    indices[candidate * 3 + 2],
                };

                assert(triIndices[0] < vertexCount);
                assert(triIndices[1] < vertexCount);
                assert(triIndices[2] < vertexCount);

                vec3 triVerts[3] =
                {
                    positions[triIndices[0]],
                    positions[triIndices[1]],
                    positions[triIndices[2]],
                };

                candidates[i].second = ComputeScore(*curr, psphere, normal, triIndices, triVerts);
            }

            // Determine whether we need to move to the next meshlet.
            if (IsMeshletFull(maxVerts, maxPrims, *curr))
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                // Use one of our existing candidates as the next meshlet seed.
                if (!candidates.empty())
                {
                    candidates[0] = candidates.back();
                    candidates.resize(1);
                    candidateCheck.insert(candidates[0].first);
                }

                output.emplace_back();
                curr = &output.back();
            }
            else
            {
                std::sort(candidates.begin(), candidates.end(), &CompareScores);
            }
        }
        else
        {
            if (candidates.empty())
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                output.emplace_back();
                curr = &output.back();
            }
        }

        // Ran out of candidates; add a new seed candidate to start the next meshlet.
        if (candidates.empty())
        {
            while (triIndex < triCount && checklist[triIndex])
                ++triIndex;

            if (triIndex == triCount)
                break;

            candidates.push_back(std::make_pair(triIndex, 0.0f));
            candidateCheck.insert(triIndex);
        }
    }

    // The last meshlet may have never had any primitives added to it - in which case we want to remove it.
    if (output.back().PrimitiveIndices.empty())
    {
        output.pop_back();
    }
}




namespace
{
    struct EdgeEntry
    {
        uint32_t   i0;
        uint32_t   i1;
        uint32_t   i2;

        uint32_t   Face;
        EdgeEntry* Next;
    };

    size_t CRCHash(const uint32_t* dwords, uint32_t dwordCount)
    {
        size_t h = 0;

        for (uint32_t i = 0; i < dwordCount; ++i)
        {
            uint32_t highOrd = h & 0xf8000000;
            h = h << 5;
            h = h ^ (highOrd >> 27);
            h = h ^ size_t(dwords[i]);
        }

        return h;
    }

    template <typename T>
    inline size_t Hash(const T& val)
    {
        return std::hash<T>()(val);
    }
}

namespace std
{
    template <> struct hash<float3> { size_t operator()(const float3& v) const { return CRCHash(reinterpret_cast<const uint32_t*>(&v), sizeof(v) / 3); } };
}


///
// External Interface

namespace internal
{
    template <typename T>
    void BuildAdjacencyList(
        const T* indices, uint32_t indexCount,
        const float3* positions, uint32_t vertexCount,
        uint32_t* adjacency);
}


void BuildAdjacencyList(
    const uint32_t* indices, uint32_t indexCount,
    const float3* positions, uint32_t vertexCount,
    uint32_t* adjacency
)
{
    internal::BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency);
}


///
// Implementation

template <typename T>
void internal::BuildAdjacencyList(
    const T* indices, uint32_t indexCount,
    const float3* positions, uint32_t vertexCount,
    uint32_t* adjacency
)
{
    const uint32_t triCount = indexCount / 3;
    // Find point reps (unique positions) in the position stream
    // Create a mapping of non-unique vertex indices to point reps
    std::vector<T> pointRep;
    pointRep.resize(vertexCount);

    std::unordered_map<size_t, T> uniquePositionMap;
    uniquePositionMap.reserve(vertexCount);

    for (uint32_t i = 0; i < vertexCount; ++i)
    {
        float3 position = *(positions + i);
        size_t hash = Hash(position);

        auto it = uniquePositionMap.find(hash);
        if (it != uniquePositionMap.end())
        {
            // Position already encountered - reference previous index
            pointRep[i] = it->second;
        }
        else
        {
            // New position found - add to hash table and LUT
            uniquePositionMap.insert(std::make_pair(hash, static_cast<T>(i)));
            pointRep[i] = static_cast<T>(i);
        }
    }

    // Create a linked list of edges for each vertex to determine adjacency
    const uint32_t hashSize = vertexCount / 3;

    std::unique_ptr<EdgeEntry* []> hashTable(new EdgeEntry * [hashSize]);
    std::unique_ptr<EdgeEntry[]> entries(new EdgeEntry[triCount * 3]);

    std::memset(hashTable.get(), 0, sizeof(EdgeEntry*) * hashSize);
    uint32_t entryIndex = 0;

    for (uint32_t iFace = 0; iFace < triCount; ++iFace)
    {
        uint32_t index = iFace * 3;

        // Create a hash entry in the hash table for each each.
        for (uint32_t iEdge = 0; iEdge < 3; ++iEdge)
        {
            T i0 = pointRep[indices[index + (iEdge % 3)]];
            T i1 = pointRep[indices[index + ((iEdge + 1) % 3)]];
            T i2 = pointRep[indices[index + ((iEdge + 2) % 3)]];

            auto& entry = entries[entryIndex++];
            entry.i0 = i0;
            entry.i1 = i1;
            entry.i2 = i2;

            uint32_t key = entry.i0 % hashSize;

            entry.Next = hashTable[key];
            entry.Face = iFace;

            hashTable[key] = &entry;
        }
    }


    // Initialize the adjacency list
    std::memset(adjacency, uint32_t(-1), indexCount * sizeof(uint32_t));

    for (uint32_t iFace = 0; iFace < triCount; ++iFace)
    {
        uint32_t index = iFace * 3;

        for (uint32_t point = 0; point < 3; ++point)
        {
            if (adjacency[iFace * 3 + point] != uint32_t(-1))
                continue;

            // Look for edges directed in the opposite direction.
            T i0 = pointRep[indices[index + ((point + 1) % 3)]];
            T i1 = pointRep[indices[index + (point % 3)]];
            T i2 = pointRep[indices[index + ((point + 2) % 3)]];

            // Find a face sharing this edge
            uint32_t key = i0 % hashSize;

            EdgeEntry* found = nullptr;
            EdgeEntry* foundPrev = nullptr;

            for (EdgeEntry* current = hashTable[key], *prev = nullptr; current != nullptr; prev = current, current = current->Next)
            {
                if (current->i1 == i1 && current->i0 == i0)
                {
                    found = current;
                    foundPrev = prev;
                    break;
                }
            }

            // Cache this face's normal
            float3 n0;
            {
                float3 p0 = positions[i1];
                float3 p1 = positions[i0];
                float3 p2 = positions[i2];

                float3 e0 = p0 - p1;
                float3 e1 = p1 - p2;

                n0 = vec3::cross(e0, e1).normalized();
            }

            // Use face normal dot product to determine best edge-sharing candidate.
            float bestDot = -2.0f;
            for (EdgeEntry* current = found, *prev = foundPrev; current != nullptr; prev = current, current = current->Next)
            {
                if (bestDot == -2.0f || (current->i1 == i1 && current->i0 == i0))
                {
                    float3 p0 = positions[current->i0];
                    float3 p1 = positions[current->i1];
                    float3 p2 = positions[current->i2];

                    float3 e0 = p0 - p1;
                    float3 e1 = p1 - p2;

                    float3 n1 = vec3::cross(e0, e1).normalized();

                    float dot = vec3::dot(n0, n1);

                    if (dot > bestDot)
                    {
                        found = current;
                        foundPrev = prev;
                        bestDot = dot;
                    }
                }
            }

            // Update hash table and adjacency list
            if (found && found->Face != uint32_t(-1))
            {
                // Erase the found from the hash table linked list.
                if (foundPrev != nullptr)
                {
                    foundPrev->Next = found->Next;
                }
                else
                {
                    hashTable[key] = found->Next;
                }

                // Update adjacency information
                adjacency[iFace * 3 + point] = found->Face;

                // Search & remove this face from the table linked list
                uint32_t key2 = i1 % hashSize;

                for (EdgeEntry* current = hashTable[key2], *prev = nullptr; current != nullptr; prev = current, current = current->Next)
                {
                    if (current->Face == iFace && current->i1 == i0 && current->i0 == i1)
                    {
                        if (prev != nullptr)
                        {
                            prev->Next = current->Next;
                        }
                        else
                        {
                            hashTable[key2] = current->Next;
                        }

                        break;
                    }
                }

                bool linked = false;
                for (uint32_t point2 = 0; point2 < point; ++point2)
                {
                    if (found->Face == adjacency[iFace * 3 + point2])
                    {
                        linked = true;
                        adjacency[iFace * 3 + point] = uint32_t(-1);
                        break;
                    }
                }

                if (!linked)
                {
                    uint32_t edge2 = 0;
                    for (; edge2 < 3; ++edge2)
                    {
                        T k = indices[found->Face * 3 + edge2];
                        if (k == uint32_t(-1))
                            continue;

                        if (pointRep[k] == i0)
                            break;
                    }

                    if (edge2 < 3)
                    {
                        adjacency[found->Face * 3 + edge2] = iFace;
                    }
                }
            }
        }
    }
}

float4 MinimumBoundingSphere(float3* points, uint32_t count)
{
    assert(points != nullptr && count != 0);

    // Find the min & max points indices along each axis.
    uint32_t minAxis[3] = { 0, 0, 0 };
    uint32_t maxAxis[3] = { 0, 0, 0 };

    for (uint32_t i = 1; i < count; ++i)
    {
        float* point = (float*)(points + i);

        for (uint32_t j = 0; j < 3; ++j)
        {
            float* min = (float*)(&points[minAxis[j]]);
            float* max = (float*)(&points[maxAxis[j]]);

            minAxis[j] = point[j] < min[j] ? i : minAxis[j];
            maxAxis[j] = point[j] > max[j] ? i : maxAxis[j];
        }
    }

    // Find axis with maximum span.
    float distSqMax = 0;
    uint32_t axis = 0;

    for (uint32_t i = 0; i < 3u; ++i)
    {
        float3 min = points[minAxis[i]];
        float3 max = points[maxAxis[i]];

        float distSq = (max - min).length_squared();
        if (distSq> distSqMax)
        {
            distSqMax = distSq;
            axis = i;
        }
    }

    // Calculate an initial starting center point & radius.
    float3 p1 = points[minAxis[axis]];
    float3 p2 = points[maxAxis[axis]];

    float3 center = (p1 + p2) * 0.5f;
    float radius = (p2 - p1).length() * 0.5f;
    float radiusSq = radius * radius;

    // Add all our points to bounding sphere expanding radius & recalculating center point as necessary.
    for (uint32_t i = 0; i < count; ++i)
    {
        float3 point = points[i];
        float distSq = (point - center).length_squared();

        if (distSq> radiusSq)
        {
            float dist = sqrt(distSq);
            float k = (radius / dist) * 0.5f + 0.5f;

            center = center * k + point * (1.0f - k);
            radius = (radius + dist) * 0.5f;
        }
    }

    return float4(center, radius);
}


inline float3 QuantizeSNorm(float3 value)
{
    return (float3::clamp(value, float3(-1), float3(1)) * 0.5f + float3(0.5f)) * 255.0f;
}

inline float3 QuantizeUNorm(float3 value)
{
    return float3::clamp(value, float3(0), float3(1)) * 255.0f;
}

static const DWORD CNORM_WIND_CW = 1;

HRESULT ComputeCullData(
    const float3* positions, uint32_t vertexCount,
    InlineMeshlet<UINT>& meshlet,
    DWORD flags
)
{
    UNREFERENCED_PARAMETER(vertexCount);

    float3 vertices[256];
    float3 normals[256];

  
        auto& m = meshlet;
        auto& c = meshlet.cull_data;

        // Cache vertices
        for (uint32_t i = 0; i < m.UniqueVertexIndices.size(); ++i)
        {
            uint32_t vIndex = m.UniqueVertexIndices[i];

            assert(vIndex < vertexCount);
            vertices[i] = positions[vIndex];
        }

        // Generate primitive normals & cache
        for (uint32_t i = 0; i < m.PrimitiveIndices.size()/3; ++i)
        {
            auto i0 = m.PrimitiveIndices[ 3*i];
            auto i1 = m.PrimitiveIndices[3 * i+1];
            auto i2 = m.PrimitiveIndices[3 * i+2];

            float3 triangle[3]
            {
               vertices[i0],
               vertices[i1],
                vertices[i2],
            };

            float3 p10 = triangle[1] - triangle[0];
            float3 p20 = triangle[2] - triangle[0];
            float3 n = vec3::cross(p10, p20).normalized();

           normals[i] = (flags & CNORM_WIND_CW) != 0 ? -n : n;
        }

        // Calculate spatial bounds
        float4 positionBounds = MinimumBoundingSphere(vertices, static_cast<UINT>(m.UniqueVertexIndices.size()));
        c.BoundingSphere= positionBounds;

        // Calculate the normal cone
        // 1. Normalized center point of minimum bounding sphere of unit normals == conic axis
        float4 normalBounds = MinimumBoundingSphere(normals, static_cast<UINT>(m.PrimitiveIndices.size()/3));

        // 2. Calculate dot product of all normals to conic axis, selecting minimum
        float3 axis = normalBounds.normalized();

        float minDot = 1;
        for (uint32_t i = 0; i < m.PrimitiveIndices.size()/3; ++i)
        {
            float dot = vec3::dot(axis, normals[i]);
            minDot = std::min(minDot, dot);
        }

        if (minDot< 0.1f)
        {
            uint8_t* cone = reinterpret_cast<uint8_t *>(&c.NormalCone);
            // Degenerate cone
            cone[0] = 127;
            cone[1] = 127;
            cone[2] = 127;
            cone[3] = 255;
            return S_OK;
        }

        // Find the point on center-t*axis ray that lies in negative half-space of all triangles
        float maxt = 0;

        for (uint32_t i = 0; i < m.PrimitiveIndices.size() / 3; ++i)
        {
      //      auto primitive = primitiveIndices[m.primitiveOffset + i];

            auto i0 = m.PrimitiveIndices[3 * i];
            auto i1 = m.PrimitiveIndices[3 * i + 1];
            auto i2 = m.PrimitiveIndices[3 * i + 2];


            uint32_t indices[3]
            {
                i0,
                i1,
                i2,
            };

            float3 triangle[3]
            {
                vertices[indices[0]],
                vertices[indices[1]],
                vertices[indices[2]],
            };

            float3 c = positionBounds.xyz - triangle[0];

            float3 n = normals[i];
            float dc = vec3::dot(c, n);
            float dn = vec3::dot(axis, n);

            // dn should be larger than mindp cutoff above
            assert(dn > 0.0f);
            float t = dc / dn;

            maxt = (t > maxt) ? t : maxt;
        }

        // cone apex should be in the negative half-space of all cluster triangles by construction
        c.ApexOffset = maxt;

        // cos(a) for normal cone is minDot; we need to add 90 degrees on both sides and invert the cone
        // which gives us -cos(a+90) = -(-sin(a)) = sin(a) = sqrt(1 - cos^2(a))
        float coneCutoff = sqrtf(1 - minDot * minDot);

        // 3. Quantize to uint8
        float3 quantized = QuantizeSNorm(axis);
        uint8_t* cone = reinterpret_cast<uint8_t*>(&c.NormalCone);

        cone[0] = (uint8_t)quantized.x;
        cone[1] = (uint8_t)quantized.y;
        cone[2] = (uint8_t)quantized.z;

        float3 error = ((quantized / 255.0f) * 2.0f - float3(1)) - axis;
        error = error.abs().sum();

        quantized = QuantizeUNorm(float3(coneCutoff) + error);
        quantized = vec3::min(quantized + float3(1), float3(255.0f));
        cone[3] = (uint8_t)quantized.x;
    

    return S_OK;
}
