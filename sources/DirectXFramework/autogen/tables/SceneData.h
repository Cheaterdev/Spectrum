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
		Render::HLSL::StructuredBuffer<node_data> nodes;
		Render::HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
		Render::HLSL::StructuredBuffer<MeshCommandData> meshes;
		Render::HLSL::StructuredBuffer<MaterialCommandData> materials;
		Render::HLSL::StructuredBuffer<MeshInstance> meshInstances;
		Render::HLSL::StructuredBuffer<Meshlet> meshlets;
		Render::HLSL::StructuredBuffer<MeshletCullData> meshletCullData;
		Render::HLSL::StructuredBuffer<uint> indices;
		std::vector<Render::HLSL::Texture2D<float4>> material_textures;
		Render::HLSL::StructuredBuffer<node_data>& GetNodes() { return nodes; }
		Render::HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return vertexes; }
		Render::HLSL::StructuredBuffer<MeshCommandData>& GetMeshes() { return meshes; }
		Render::HLSL::StructuredBuffer<MaterialCommandData>& GetMaterials() { return materials; }
		Render::HLSL::StructuredBuffer<MeshInstance>& GetMeshInstances() { return meshInstances; }
		Render::HLSL::StructuredBuffer<Meshlet>& GetMeshlets() { return meshlets; }
		Render::HLSL::StructuredBuffer<MeshletCullData>& GetMeshletCullData() { return meshletCullData; }
		Render::HLSL::StructuredBuffer<uint>& GetIndices() { return indices; }
		std::vector<Render::HLSL::Texture2D<float4>>& GetMaterial_textures() { return material_textures; }
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
			compiler.compile(material_textures);
		}
	};
	#pragma pack(pop)
}
