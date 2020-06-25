#pragma once
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
			Render::Handle commands;
			Render::Handle nodes;
			Render::Handle vertexes;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		Render::Handle& GetCommands() { return srv.commands; }
		Render::Handle& GetNodes() { return srv.nodes; }
		Render::Handle& GetVertexes() { return srv.vertexes; }
		Render::Bindless& GetMaterial_textures() { return bindless; }
		SceneData(SRV&srv,Render::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
