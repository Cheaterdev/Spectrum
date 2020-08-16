#pragma once
namespace Table 
{
	#pragma pack(push, 1)
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
	#pragma pack(pop)
}
