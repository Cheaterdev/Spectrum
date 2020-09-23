#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct ColorRect
	{
		struct CB
		{
			float4 pos[2];
			float4 color;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4* GetPos() { return cb.pos; }
		float4& GetColor() { return cb.color; }
		ColorRect(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
