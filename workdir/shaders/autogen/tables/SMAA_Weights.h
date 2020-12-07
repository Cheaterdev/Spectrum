#pragma once
struct SMAA_Weights_srv
{
	Texture2D<float4> areaTex;
	Texture2D<float4> searchTex;
	Texture2D<float4> edgesTex;
};
struct SMAA_Weights
{
	SMAA_Weights_srv srv;
	Texture2D<float4> GetAreaTex() { return srv.areaTex; }
	Texture2D<float4> GetSearchTex() { return srv.searchTex; }
	Texture2D<float4> GetEdgesTex() { return srv.edgesTex; }

};
 const SMAA_Weights CreateSMAA_Weights(SMAA_Weights_srv srv)
{
	const SMAA_Weights result = {srv
	};
	return result;
}
