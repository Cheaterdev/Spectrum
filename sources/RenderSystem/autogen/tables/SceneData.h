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
			DX12::HLSL::StructuredBuffer<node_data> nodes;
			DX12::HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
			DX12::HLSL::StructuredBuffer<MeshCommandData> meshes;
			DX12::HLSL::StructuredBuffer<MaterialCommandData> materials;
			DX12::HLSL::StructuredBuffer<MeshInstance> meshInstances;
		} &srv;
		DX12::Bindless& bindless;
		DX12::HLSL::StructuredBuffer<node_data>& GetNodes() { return srv.nodes; }
		DX12::HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return srv.vertexes; }
		DX12::HLSL::StructuredBuffer<MeshCommandData>& GetMeshes() { return srv.meshes; }
		DX12::HLSL::StructuredBuffer<MaterialCommandData>& GetMaterials() { return srv.materials; }
		DX12::HLSL::StructuredBuffer<MeshInstance>& GetMeshInstances() { return srv.meshInstances; }
		DX12::Bindless& GetMaterial_textures() { return bindless; }
		SceneData(SRV&srv,DX12::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
