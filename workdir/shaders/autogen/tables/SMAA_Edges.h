#pragma once
struct SMAA_Edges_cb
{
	float4 subsampleIndices;
	float4 SMAA_RT_METRICS;
};
struct SMAA_Edges_srv
{
	Texture2D areaTex;
	Texture2D searchTex;
	Texture2D colorTex;
};
struct SMAA_Edges
{
	SMAA_Edges_cb cb;
	SMAA_Edges_srv srv;
	Texture2D GetAreaTex() { return srv.areaTex; }
	Texture2D GetSearchTex() { return srv.searchTex; }
	Texture2D GetColorTex() { return srv.colorTex; }
	float4 GetSubsampleIndices() { return cb.subsampleIndices; }
	float4 GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }

};
 const SMAA_Edges CreateSMAA_Edges(SMAA_Edges_cb cb,SMAA_Edges_srv srv)
{
	const SMAA_Edges result = {cb,srv
	};
	return result;
}
