#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
#include "MeshInstance.h"
#include "Meshlet.h"
#include "MeshletCullData.h"
#include "mesh_vertex_input.h"
#include "node_data.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct SceneData
	{
		HLSL::StructuredBuffer<node_data> nodes;
		HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
		HLSL::StructuredBuffer<MeshCommandData> meshes;
		HLSL::StructuredBuffer<MaterialCommandData> materials;
		HLSL::StructuredBuffer<MeshInstance> meshInstances;
		HLSL::StructuredBuffer<Meshlet> meshlets;
		HLSL::StructuredBuffer<MeshletCullData> meshletCullData;
		HLSL::StructuredBuffer<uint> indices;
		HLSL::StructuredBuffer<node_data>& GetNodes() { return nodes; }
		HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return vertexes; }
		HLSL::StructuredBuffer<MeshCommandData>& GetMeshes() { return meshes; }
		HLSL::StructuredBuffer<MaterialCommandData>& GetMaterials() { return materials; }
		HLSL::StructuredBuffer<MeshInstance>& GetMeshInstances() { return meshInstances; }
		HLSL::StructuredBuffer<Meshlet>& GetMeshlets() { return meshlets; }
		HLSL::StructuredBuffer<MeshletCullData>& GetMeshletCullData() { return meshletCullData; }
		HLSL::StructuredBuffer<uint>& GetIndices() { return indices; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(nodes);
			compiler.compile(vertexes);
			compiler.compile(meshes);
			compiler.compile(materials);
			compiler.compile(meshInstances);
			compiler.compile(meshlets);
			compiler.compile(meshletCullData);
			compiler.compile(indices);
		}
	};
	#pragma pack(pop)
}
