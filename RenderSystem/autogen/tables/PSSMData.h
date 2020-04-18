#pragma once
#include "Camera.h"
namespace Table 
{
	struct PSSMData
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle light_buffer;
			Render::Handle light_cameras;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetLight_buffer() { return srv.light_buffer; }
		Render::Handle& GetLight_cameras() { return srv.light_cameras; }
		PSSMData(SRV&srv) :srv(srv){}
	};
}
