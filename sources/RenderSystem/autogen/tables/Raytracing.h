#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Raytracing
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle scene;
			Render::Handle index_buffer;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetScene() { return srv.scene; }
		Render::Handle& GetIndex_buffer() { return srv.index_buffer; }
		Raytracing(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
