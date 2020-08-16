#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferQuality
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle ref;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetRef() { return srv.ref; }
		GBufferQuality(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
