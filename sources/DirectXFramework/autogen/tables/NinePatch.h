#pragma once
#include "vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct NinePatch
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<vertex_input> vb;
		} &srv;
		Render::Bindless& bindless;
		Render::HLSL::StructuredBuffer<vertex_input>& GetVb() { return srv.vb; }
		Render::Bindless& GetTextures() { return bindless; }
		NinePatch(SRV&srv,Render::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
