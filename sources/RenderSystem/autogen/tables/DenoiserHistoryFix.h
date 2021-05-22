#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserHistoryFix
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> color;
			Render::HLSL::Texture2D<float> frames;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture2D<float4> target;
		} &uav;
		Render::HLSL::Texture2D<float4>& GetColor() { return srv.color; }
		Render::HLSL::Texture2D<float>& GetFrames() { return srv.frames; }
		Render::HLSL::RWTexture2D<float4>& GetTarget() { return uav.target; }
		DenoiserHistoryFix(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
