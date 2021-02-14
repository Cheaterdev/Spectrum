#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelOutput
	{
		struct UAV
		{
			Render::HLSL::RWTexture2D<float4> noise;
			Render::HLSL::RWTexture2D<float> frames;
		} &uav;
		Render::HLSL::RWTexture2D<float4>& GetNoise() { return uav.noise; }
		Render::HLSL::RWTexture2D<float>& GetFrames() { return uav.frames; }
		VoxelOutput(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
