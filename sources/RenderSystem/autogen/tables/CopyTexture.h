#pragma once
namespace Table 
{
	#pragma pack(push, 1)
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
	#pragma pack(pop)
}
