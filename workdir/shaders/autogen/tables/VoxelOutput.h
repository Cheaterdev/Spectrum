#pragma once
struct VoxelOutput_uav
{
	uint noise; // RWTexture2D<float4>
	uint frames; // RWTexture2D<float>
	uint DirAndPdf; // RWTexture2D<float4>
};
struct VoxelOutput
{
	VoxelOutput_uav uav;
	RWTexture2D<float4> GetNoise() { return ResourceDescriptorHeap[uav.noise]; }
	RWTexture2D<float> GetFrames() { return ResourceDescriptorHeap[uav.frames]; }
	RWTexture2D<float4> GetDirAndPdf() { return ResourceDescriptorHeap[uav.DirAndPdf]; }

};
 const VoxelOutput CreateVoxelOutput(VoxelOutput_uav uav)
{
	const VoxelOutput result = {uav
	};
	return result;
}
