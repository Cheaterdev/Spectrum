#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct EnvFilter
	{
		struct CB
		{
			uint4 face;
			float4 scaler;
			uint4 size;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint4& GetFace() { return cb.face; }
		float4& GetScaler() { return cb.scaler; }
		uint4& GetSize() { return cb.size; }
		EnvFilter(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
