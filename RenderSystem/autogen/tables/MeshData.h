#pragma once
#include "mesh_vertex_input.h"
namespace Table 
{
	struct MeshData
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle vb;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetVb() { return srv.vb; }
		MeshData(SRV&srv) :srv(srv){}
	};
}
