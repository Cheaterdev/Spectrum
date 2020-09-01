#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Weights
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle areaTex;
			Render::Handle searchTex;
			Render::Handle edgesTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetAreaTex() { return srv.areaTex; }
		Render::Handle& GetSearchTex() { return srv.searchTex; }
		Render::Handle& GetEdgesTex() { return srv.edgesTex; }
		SMAA_Weights(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
