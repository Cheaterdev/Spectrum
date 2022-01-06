#pragma once
struct VoxelBlur
{
	uint noisy_output; // Texture2D<float4>
	uint hit_and_pdf; // Texture2D<float4>
	uint prev_result; // Texture2D<float4>
	uint screen_result; // RWTexture2D<float4>
	uint gi_result; // RWTexture2D<float4>
	Texture2D<float4> GetNoisy_output() { return ResourceDescriptorHeap[noisy_output]; }
	Texture2D<float4> GetHit_and_pdf() { return ResourceDescriptorHeap[hit_and_pdf]; }
	Texture2D<float4> GetPrev_result() { return ResourceDescriptorHeap[prev_result]; }
	RWTexture2D<float4> GetScreen_result() { return ResourceDescriptorHeap[screen_result]; }
	RWTexture2D<float4> GetGi_result() { return ResourceDescriptorHeap[gi_result]; }
};
