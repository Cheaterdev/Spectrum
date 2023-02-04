#pragma once
struct DenoiserShadow_Fileter
{
	float4x4 ProjectionInverse; // float4x4
	int2 BufferDimensions; // int2
	float2 InvBufferDimensions; // float2
	float DepthSimilaritySigma; // float
	uint t2d_DepthBuffer; // Texture2D<float>
	uint t2d_NormalBuffer; // Texture2D<float16_t3>
	uint sb_tileMetaData; // StructuredBuffer<uint>
	uint rqt2d_input; // Texture2D<float16_t2>
	uint rwt2d_history; // RWTexture2D<float2>
	uint rwt2d_output; // RWTexture2D<unormfloat4>
	float4x4 GetProjectionInverse() { return ProjectionInverse; }
	int2 GetBufferDimensions() { return BufferDimensions; }
	float2 GetInvBufferDimensions() { return InvBufferDimensions; }
	float GetDepthSimilaritySigma() { return DepthSimilaritySigma; }
	Texture2D<float> GetT2d_DepthBuffer() { return ResourceDescriptorHeap[t2d_DepthBuffer]; }
	Texture2D<float16_t3> GetT2d_NormalBuffer() { return ResourceDescriptorHeap[t2d_NormalBuffer]; }
	StructuredBuffer<uint> GetSb_tileMetaData() { return ResourceDescriptorHeap[sb_tileMetaData]; }
	Texture2D<float16_t2> GetRqt2d_input() { return ResourceDescriptorHeap[rqt2d_input]; }
	RWTexture2D<float2> GetRwt2d_history() { return ResourceDescriptorHeap[rwt2d_history]; }
	RWTexture2D<unormfloat4> GetRwt2d_output() { return ResourceDescriptorHeap[rwt2d_output]; }
};
