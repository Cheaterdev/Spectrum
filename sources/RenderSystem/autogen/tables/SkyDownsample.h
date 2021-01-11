#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SkyDownsample
	{
		struct CB
		{
			float4 scaler;
			uint4 size;
		} &cb;
		struct SRV
		{
			DX12::Handle sourceTex;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		DX12::Handle& GetSourceTex() { return srv.sourceTex; }
		float4& GetScaler() { return cb.scaler; }
		uint4& GetSize() { return cb.size; }
		SkyDownsample(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
