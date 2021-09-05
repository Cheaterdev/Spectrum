//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "pch.h"

///
// External Interface

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
    const uint16_t* indices, UINT indexCount,
    const vec3* positions, UINT vertexCount,
    std::vector<InlineMeshlet<uint16_t>>& output
)
{
    return internal::Meshletize(maxVerts, maxPrims, indices, indexCount, positions, vertexCount, output);
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
    typename InlineMeshlet<T>::PackedTriangle prim = {};
    prim.i0 = indices[0];
    prim.i1 = indices[1];
    prim.i2 = indices[2];

    meshlet.PrimitiveIndices.push_back(prim);

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

    //BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency.data());

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
          /////////////////////////////  psphere = MinimumBoundingSphere(m_positions.data(), static_cast<UINT>(m_positions.size()));

            float4 nsphere;///////////////////////// = MinimumBoundingSphere(normals.data(), static_cast<UINT>(normals.size()));
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
