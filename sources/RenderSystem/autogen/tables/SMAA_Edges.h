#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Edges
	{
		struct CB
		{
			float4 subsampleIndices;
			float4 SMAA_RT_METRICS;
		} &cb;
		struct SRV
		{
			Render::Handle areaTex;
			Render::Handle searchTex;
			Render::Handle colorTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetAreaTex() { return srv.areaTex; }
		Render::Handle& GetSearchTex() { return srv.searchTex; }
		Render::Handle& GetColorTex() { return srv.colorTex; }
		float4& GetSubsampleIndices() { return cb.subsampleIndices; }
		float4& GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }
		SMAA_Edges(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
