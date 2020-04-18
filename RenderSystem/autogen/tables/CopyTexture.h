#pragma once
namespace Table 
{
	struct CopyTexture
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle srcTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetSrcTex() { return srv.srcTex; }
		CopyTexture(SRV&srv) :srv(srv){}
	};
}
