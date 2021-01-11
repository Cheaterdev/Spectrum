#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Edges
	{
		using CB = Empty;
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
		Edges(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
