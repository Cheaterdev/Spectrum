#pragma once
struct VoxelOutput_uav
{
	RWTexture2D<float4> noise;
	RWTexture2D<float> frames;
	RWTexture2D<float4> DirAndPdf;
};
struct VoxelOutput
{
	VoxelOutput_uav uav;
	RWTexture2D<float4> GetNoise() { return uav.noise; }
	RWTexture2D<float> GetFrames() { return uav.frames; }
	RWTexture2D<float4> GetDirAndPdf() { return uav.DirAndPdf; }

};
 const VoxelOutput CreateVoxelOutput(VoxelOutput_uav uav)
{
	const VoxelOutput result = {uav
	};
	return result;
}
