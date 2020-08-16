#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMDataGlobal
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle light_buffer;
			Render::Handle light_camera;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetLight_buffer() { return srv.light_buffer; }
		Render::Handle& GetLight_camera() { return srv.light_camera; }
		PSSMDataGlobal(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
