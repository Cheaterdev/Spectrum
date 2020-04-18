#pragma once
namespace Table 
{
	struct TextureRenderer
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle texture;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetTexture() { return srv.texture; }
		TextureRenderer(SRV&srv) :srv(srv){}
	};
}
