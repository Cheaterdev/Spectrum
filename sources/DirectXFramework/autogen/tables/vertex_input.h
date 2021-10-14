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
			float4 mulColor;
			float4 addColor;
		} &cb;
		float2& GetPos() { return cb.pos; }
		float2& GetTc() { return cb.tc; }
		float4& GetMulColor() { return cb.mulColor; }
		float4& GetAddColor() { return cb.addColor; }
		vertex_input(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
