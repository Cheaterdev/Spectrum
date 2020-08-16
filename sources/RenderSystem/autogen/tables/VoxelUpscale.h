#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelUpscale
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle tex_downsampled;
			Render::Handle tex_gi_prev;
			Render::Handle tex_depth_prev;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetTex_downsampled() { return srv.tex_downsampled; }
		Render::Handle& GetTex_gi_prev() { return srv.tex_gi_prev; }
		Render::Handle& GetTex_depth_prev() { return srv.tex_depth_prev; }
		VoxelUpscale(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
