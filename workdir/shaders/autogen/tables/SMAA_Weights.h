#pragma once
struct SMAA_Weights
{
	uint areaTex; // Texture2D<float4>
	uint searchTex; // Texture2D<float4>
	uint edgesTex; // Texture2D<float4>
	Texture2D<float4> GetAreaTex() { return ResourceDescriptorHeap[areaTex]; }
	Texture2D<float4> GetSearchTex() { return ResourceDescriptorHeap[searchTex]; }
	Texture2D<float4> GetEdgesTex() { return ResourceDescriptorHeap[edgesTex]; }
};
