#pragma once
struct VoxelOutput
{
	uint noise; // RWTexture2D<float4>
	uint frames; // RWTexture2D<float>
	uint DirAndPdf; // RWTexture2D<float4>
	RWTexture2D<float4> GetNoise() { return ResourceDescriptorHeap[noise]; }
	RWTexture2D<float> GetFrames() { return ResourceDescriptorHeap[frames]; }
	RWTexture2D<float4> GetDirAndPdf() { return ResourceDescriptorHeap[DirAndPdf]; }
};
