#pragma once
#include "mesh_vertex_input.h"
#include "node_data.h"
namespace Table 
{
	struct MeshData
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle vb;
			Render::Handle nodes;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetVb() { return srv.vb; }
		Render::Handle& GetNodes() { return srv.nodes; }
		MeshData(SRV&srv) :srv(srv){}
	};
}
