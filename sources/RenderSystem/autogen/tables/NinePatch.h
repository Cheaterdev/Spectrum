#pragma once
#include "vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct NinePatch
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle vb;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		Render::Handle& GetVb() { return srv.vb; }
		Render::Bindless& GetTextures() { return bindless; }
		NinePatch(SRV&srv,Render::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
