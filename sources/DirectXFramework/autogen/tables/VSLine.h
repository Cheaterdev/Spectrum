#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VSLine
	{
		struct CB
		{
			float2 pos;
			float4 color;
		} &cb;
		float2& GetPos() { return cb.pos; }
		float4& GetColor() { return cb.color; }
		VSLine(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
