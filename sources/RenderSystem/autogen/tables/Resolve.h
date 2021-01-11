#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Resolve
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
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		DX12::Handle& GetAreaTex() { return srv.areaTex; }
		DX12::Handle& GetSearchTex() { return srv.searchTex; }
		float4& GetSubsampleIndices() { return cb.subsampleIndices; }
		float4& GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }
		Resolve(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
