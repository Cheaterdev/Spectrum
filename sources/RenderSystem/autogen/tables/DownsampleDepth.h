#pragma once
namespace Table 
{
	#pragma pack(push, 1)
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
	#pragma pack(pop)
}
