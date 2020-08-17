#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawStencil
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle vertices;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetVertices() { return srv.vertices; }
		DrawStencil(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
