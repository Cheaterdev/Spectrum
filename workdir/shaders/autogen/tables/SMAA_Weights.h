#pragma once
struct SMAA_Weights_srv
{
	Texture2D areaTex;
	Texture2D searchTex;
	Texture2D edgesTex;
};
struct SMAA_Weights
{
	SMAA_Weights_srv srv;
	Texture2D GetAreaTex() { return srv.areaTex; }
	Texture2D GetSearchTex() { return srv.searchTex; }
	Texture2D GetEdgesTex() { return srv.edgesTex; }

};
 const SMAA_Weights CreateSMAA_Weights(SMAA_Weights_srv srv)
{
	const SMAA_Weights result = {srv
	};
	return result;
}
