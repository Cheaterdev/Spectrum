#pragma once
namespace Table 
{
	struct UIRender
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle tex0;
			Render::Handle positions;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetTex0() { return srv.tex0; }
		Render::Handle& GetPositions() { return srv.positions; }
		UIRender(SRV&srv) :srv(srv){}
	};
}
