#pragma once
namespace Table 
{
	#pragma pack(push, 1)
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
	#pragma pack(pop)
}
