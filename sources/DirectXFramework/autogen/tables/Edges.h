#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Edges
	{
		using CB = Empty;
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
		Edges(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
