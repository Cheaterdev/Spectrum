#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameClassification
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float> frames;
		} &srv;
		struct UAV
		{
			Render::HLSL::AppendStructuredBuffer<uint2> hi;
			Render::HLSL::AppendStructuredBuffer<uint2> low;
		} &uav;
		Render::HLSL::Texture2D<float>& GetFrames() { return srv.frames; }
		Render::HLSL::AppendStructuredBuffer<uint2>& GetHi() { return uav.hi; }
		Render::HLSL::AppendStructuredBuffer<uint2>& GetLow() { return uav.low; }
		FrameClassification(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
