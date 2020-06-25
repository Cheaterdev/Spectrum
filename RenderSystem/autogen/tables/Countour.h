#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Countour
	{
		struct CB
		{
			float4 color;
		} &cb;
		struct SRV
		{
			Render::Handle tex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetColor() { return cb.color; }
		Render::Handle& GetTex() { return srv.tex; }
		Countour(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
