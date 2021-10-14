#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DownsampleDepth
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float> srcTex;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture2D<float> targetTex;
		} &uav;
		Render::HLSL::Texture2D<float>& GetSrcTex() { return srv.srcTex; }
		Render::HLSL::RWTexture2D<float>& GetTargetTex() { return uav.targetTex; }
		DownsampleDepth(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
