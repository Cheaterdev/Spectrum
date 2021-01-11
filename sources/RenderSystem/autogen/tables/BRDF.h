#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct BRDF
	{
		struct UAV
		{
			DX12::HLSL::RWTexture3D<float4> output;
		} &uav;
		DX12::HLSL::RWTexture3D<float4>& GetOutput() { return uav.output; }
		BRDF(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
