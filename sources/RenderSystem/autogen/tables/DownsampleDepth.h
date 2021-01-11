#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DownsampleDepth
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float> srcTex;
		} &srv;
		struct UAV
		{
			DX12::HLSL::RWTexture2D<float> targetTex;
		} &uav;
		DX12::HLSL::Texture2D<float>& GetSrcTex() { return srv.srcTex; }
		DX12::HLSL::RWTexture2D<float>& GetTargetTex() { return uav.targetTex; }
		DownsampleDepth(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
