#pragma once
struct SMAA_Weights_srv
{
	uint areaTex; // Texture2D<float4>
	uint searchTex; // Texture2D<float4>
	uint edgesTex; // Texture2D<float4>
};
struct SMAA_Weights
{
	SMAA_Weights_srv srv;
	Texture2D<float4> GetAreaTex() { return ResourceDescriptorHeap[srv.areaTex]; }
	Texture2D<float4> GetSearchTex() { return ResourceDescriptorHeap[srv.searchTex]; }
	Texture2D<float4> GetEdgesTex() { return ResourceDescriptorHeap[srv.edgesTex]; }

};
 const SMAA_Weights CreateSMAA_Weights(SMAA_Weights_srv srv)
{
	const SMAA_Weights result = {srv
	};
	return result;
}
