#pragma once
namespace Table 
{
	struct DownsampleDepth
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle srcTex;
		} &srv;
		struct UAV
		{
			Render::Handle targetTex;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetSrcTex() { return srv.srcTex; }
		Render::Handle& GetTargetTex() { return uav.targetTex; }
		DownsampleDepth(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
}
