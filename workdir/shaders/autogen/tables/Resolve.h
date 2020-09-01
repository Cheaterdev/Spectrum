#pragma once
struct Resolve_cb
{
	float4 subsampleIndices;
	float4 SMAA_RT_METRICS;
};
struct Resolve_srv
{
	Texture2D areaTex;
	Texture2D searchTex;
};
struct Resolve
{
	Resolve_cb cb;
	Resolve_srv srv;
	Texture2D GetAreaTex() { return srv.areaTex; }
	Texture2D GetSearchTex() { return srv.searchTex; }
	float4 GetSubsampleIndices() { return cb.subsampleIndices; }
	float4 GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }

};
 const Resolve CreateResolve(Resolve_cb cb,Resolve_srv srv)
{
	const Resolve result = {cb,srv
	};
	return result;
}
