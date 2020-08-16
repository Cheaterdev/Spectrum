#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct vertex_input
	{
		struct CB
		{
			float2 pos;
			float2 tc;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float2& GetPos() { return cb.pos; }
		float2& GetTc() { return cb.tc; }
		vertex_input(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
