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
			DX12::Handle areaTex;
			DX12::Handle searchTex;
			DX12::Handle colorTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		DX12::Handle& GetAreaTex() { return srv.areaTex; }
		DX12::Handle& GetSearchTex() { return srv.searchTex; }
		DX12::Handle& GetColorTex() { return srv.colorTex; }
		float4& GetSubsampleIndices() { return cb.subsampleIndices; }
		float4& GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }
		SMAA_Edges(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
