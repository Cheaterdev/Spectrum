#pragma once
struct DenoiserShadow_Filter
{
	float4x4 ProjectionInverse; // float4x4
	int2 BufferDimensions; // int2
	float2 InvBufferDimensions; // float2
	float DepthSimilaritySigma; // float
	uint t2d_DepthBuffer; // Texture2D<float>
	uint t2d_NormalBuffer; // Texture2D<float16_t3>
	uint sb_tileMetaData; // StructuredBuffer<uint>
	float4x4 GetProjectionInverse() { return ProjectionInverse; }
	int2 GetBufferDimensions() { return BufferDimensions; }
	float2 GetInvBufferDimensions() { return InvBufferDimensions; }
	float GetDepthSimilaritySigma() { return DepthSimilaritySigma; }
	Texture2D<float> GetT2d_DepthBuffer() { return ResourceDescriptorHeap[t2d_DepthBuffer]; }
	Texture2D<float16_t3> GetT2d_NormalBuffer() { return ResourceDescriptorHeap[t2d_NormalBuffer]; }
	StructuredBuffer<uint> GetSb_tileMetaData() { return ResourceDescriptorHeap[sb_tileMetaData]; }
};
