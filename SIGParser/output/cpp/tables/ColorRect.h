#pragma once
namespace Table 
{
	struct ColorRect
	{
		struct CB
		{
			float4 color;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetColor() { return cb.color; }
		ColorRect(CB&cb) :cb(cb){}
	};
}
