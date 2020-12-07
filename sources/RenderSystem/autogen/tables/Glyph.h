#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Glyph
	{
		struct CB
		{
			float2 pos;
			uint index;
			float4 color;
		} &cb;
		float2& GetPos() { return cb.pos; }
		uint& GetIndex() { return cb.index; }
		float4& GetColor() { return cb.color; }
		Glyph(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
