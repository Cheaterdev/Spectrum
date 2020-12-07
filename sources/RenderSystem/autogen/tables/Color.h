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
		float4& GetColor() { return cb.color; }
		Color(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
