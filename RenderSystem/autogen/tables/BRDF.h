#pragma once
namespace Table 
{
	struct BRDF
	{
		using CB = Empty;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle output;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetOutput() { return uav.output; }
		BRDF(UAV&uav) :uav(uav){}
	};
}
