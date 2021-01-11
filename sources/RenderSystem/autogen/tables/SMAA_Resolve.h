#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Resolve
	{
		using CB = Empty;
		struct SRV
		{
			DX12::Handle blendTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		DX12::Handle& GetBlendTex() { return srv.blendTex; }
		SMAA_Resolve(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
