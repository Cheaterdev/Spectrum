#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelBlur
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle tex_color;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetTex_color() { return srv.tex_color; }
		VoxelBlur(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
