#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Color
	{
		struct CB
		{
			float4 color;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetColor() { return cb.color; }
		Color(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
