#pragma once
struct DenoiserShadow_TileClassification
{
	float3 Eye; // float3
	int FirstFrame; // int
	int2 BufferDimensions; // int2
	float2 InvBufferDimensions; // float2
	float4x4 ProjectionInverse; // float4x4
	float4x4 ReprojectionMatrix; // float4x4
	float4x4 ViewProjectionInverse; // float4x4
	float DepthSimilaritySigma; // float
	uint t2d_depth; // Texture2D<float>
	uint t2d_velocity; // Texture2D<float2>
	uint t2d_normal; // Texture2D<float3>
	uint t2d_history; // Texture2D<float2>
	uint t2d_previousDepth; // Texture2D<float>
	uint sb_raytracerResult; // StructuredBuffer<uint>
	uint t2d_previousMoments; // Texture2D<float3>
	uint rwsb_tileMetaData; // RWStructuredBuffer<uint>
	uint rwt2d_reprojectionResults; // RWTexture2D<float2>
	uint rwt2d_momentsBuffer; // RWTexture2D<float3>
	float3 GetEye() { return Eye; }
	int GetFirstFrame() { return FirstFrame; }
	int2 GetBufferDimensions() { return BufferDimensions; }
	float2 GetInvBufferDimensions() { return InvBufferDimensions; }
	float4x4 GetProjectionInverse() { return ProjectionInverse; }
	float4x4 GetReprojectionMatrix() { return ReprojectionMatrix; }
	float4x4 GetViewProjectionInverse() { return ViewProjectionInverse; }
	float GetDepthSimilaritySigma() { return DepthSimilaritySigma; }
	Texture2D<float> GetT2d_depth() { return ResourceDescriptorHeap[t2d_depth]; }
	Texture2D<float2> GetT2d_velocity() { return ResourceDescriptorHeap[t2d_velocity]; }
	Texture2D<float3> GetT2d_normal() { return ResourceDescriptorHeap[t2d_normal]; }
	Texture2D<float2> GetT2d_history() { return ResourceDescriptorHeap[t2d_history]; }
	Texture2D<float> GetT2d_previousDepth() { return ResourceDescriptorHeap[t2d_previousDepth]; }
	StructuredBuffer<uint> GetSb_raytracerResult() { return ResourceDescriptorHeap[sb_raytracerResult]; }
	RWStructuredBuffer<uint> GetRwsb_tileMetaData() { return ResourceDescriptorHeap[rwsb_tileMetaData]; }
	RWTexture2D<float2> GetRwt2d_reprojectionResults() { return ResourceDescriptorHeap[rwt2d_reprojectionResults]; }
	Texture2D<float3> GetT2d_previousMoments() { return ResourceDescriptorHeap[t2d_previousMoments]; }
	RWTexture2D<float3> GetRwt2d_momentsBuffer() { return ResourceDescriptorHeap[rwt2d_momentsBuffer]; }
};
