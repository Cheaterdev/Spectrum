#pragma once
namespace Table 
{
	struct FlowGraphLines
	{
		struct CB
		{
			float2 inv_pixel;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float2& GetInv_pixel() { return cb.inv_pixel; }
		FlowGraphLines(CB&cb) :cb(cb){}
	};
}
