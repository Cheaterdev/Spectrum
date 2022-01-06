#pragma once
struct MipMapping
{
	uint SrcMipLevel; // uint
	uint NumMipLevels; // uint
	float2 TexelSize; // float2
	uint SrcMip; // Texture2D<float4>
	uint OutMip[4]; // RWTexture2D<float4>
	uint GetSrcMipLevel() { return SrcMipLevel; }
	uint GetNumMipLevels() { return NumMipLevels; }
	float2 GetTexelSize() { return TexelSize; }
	RWTexture2D<float4> GetOutMip(int i) { return ResourceDescriptorHeap[OutMip[i]]; }
	Texture2D<float4> GetSrcMip() { return ResourceDescriptorHeap[SrcMip]; }
};
