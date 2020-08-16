#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct EnvSource
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle sourceTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetSourceTex() { return srv.sourceTex; }
		EnvSource(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
