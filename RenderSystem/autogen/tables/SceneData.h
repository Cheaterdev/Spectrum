#pragma once
#include "node_data.h"
namespace Table 
{
	struct SceneData
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle nodes;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		Render::Handle& GetNodes() { return srv.nodes; }
		Render::Bindless& GetMaterial_textures() { return bindless; }
		SceneData(SRV&srv,Render::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
}
