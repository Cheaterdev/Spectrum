#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Blend
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle blendTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetBlendTex() { return srv.blendTex; }
		SMAA_Blend(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
