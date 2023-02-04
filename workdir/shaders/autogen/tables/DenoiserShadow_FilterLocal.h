#pragma once
struct DenoiserShadow_FilterLocal
{
	uint rqt2d_input; // Texture2D<float16_t2>
	uint rwt2d_output; // RWTexture2D<float2>
	Texture2D<float16_t2> GetRqt2d_input() { return ResourceDescriptorHeap[rqt2d_input]; }
	RWTexture2D<float2> GetRwt2d_output() { return ResourceDescriptorHeap[rwt2d_output]; }
};
