#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
#include "MeshInstance.h"
#include "mesh_vertex_input.h"
#include "node_data.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct SceneData
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<node_data> nodes;
			Render::HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
			Render::HLSL::StructuredBuffer<MeshCommandData> meshes;
			Render::HLSL::StructuredBuffer<MaterialCommandData> materials;
			Render::HLSL::StructuredBuffer<MeshInstance> meshInstances;
		} &srv;
		Render::Bindless& bindless;
		Render::HLSL::StructuredBuffer<node_data>& GetNodes() { return srv.nodes; }
		Render::HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return srv.vertexes; }
		Render::HLSL::StructuredBuffer<MeshCommandData>& GetMeshes() { return srv.meshes; }
		Render::HLSL::StructuredBuffer<MaterialCommandData>& GetMaterials() { return srv.materials; }
		Render::HLSL::StructuredBuffer<MeshInstance>& GetMeshInstances() { return srv.meshInstances; }
		Render::Bindless& GetMaterial_textures() { return bindless; }
		SceneData(SRV&srv,Render::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
