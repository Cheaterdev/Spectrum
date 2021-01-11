#pragma once
#include "vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct NinePatch
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<vertex_input> vb;
		} &srv;
		DX12::Bindless& bindless;
		DX12::HLSL::StructuredBuffer<vertex_input>& GetVb() { return srv.vb; }
		DX12::Bindless& GetTextures() { return bindless; }
		NinePatch(SRV&srv,DX12::Bindless &bindless) :srv(srv),bindless(bindless){}
	};
	#pragma pack(pop)
}
