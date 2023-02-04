#pragma once
struct DenoiserShadow_Prepare
{
	int2 BufferDimensions; // int2
	uint t2d_hitMaskResults; // Texture2D<uint>
	uint rwsb_shadowMask; // RWStructuredBuffer<uint>
	int2 GetBufferDimensions() { return BufferDimensions; }
	Texture2D<uint> GetT2d_hitMaskResults() { return ResourceDescriptorHeap[t2d_hitMaskResults]; }
	RWStructuredBuffer<uint> GetRwsb_shadowMask() { return ResourceDescriptorHeap[rwsb_shadowMask]; }
};
