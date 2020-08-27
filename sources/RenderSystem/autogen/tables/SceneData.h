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
		using CB = Empty;
		struct SRV
		{
			Render::Handle nodes;
			Render::Handle vertexes;
			Render::Handle meshes;
			Render::Handle materials;
			Render::Handle meshInstances;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		Render::Handle& GetNodes() { return srv.nodes; }
		Render::Handle& GetVertexes() { return srv.vertexes; }
		Render::Handle& GetMeshes() { return srv.meshes; }
		Render::Handle& GetMaterials() { return srv.materials; }
		Render::Handle& GetMeshInstances() { return srv.meshInstances; }
		Render::Bindless& GetMaterial_textures() { return bindless; }
		SceneData(SRV&srv,Render::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
