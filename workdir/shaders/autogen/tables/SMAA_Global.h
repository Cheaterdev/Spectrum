#pragma once
struct SMAA_Global
{
	float4 subsampleIndices; // float4
	float4 SMAA_RT_METRICS; // float4
	uint colorTex; // Texture2D<float4>
	float4 GetSubsampleIndices() { return subsampleIndices; }
	float4 GetSMAA_RT_METRICS() { return SMAA_RT_METRICS; }
	Texture2D<float4> GetColorTex() { return ResourceDescriptorHeap[colorTex]; }
};
