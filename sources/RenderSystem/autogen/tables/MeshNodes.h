#pragma once
#include "mesh_vertex_input.h"
#include "node_data.h"
namespace Table 
{
	struct MeshNodes
	{
		using CB = Empty;
		struct SRV
		{
			DX12::Handle vb;
			DX12::Handle nodes;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		DX12::Handle& GetVb() { return srv.vb; }
		DX12::Handle& GetNodes() { return srv.nodes; }
		MeshNodes(SRV&srv) :srv(srv){}
	};
}
