#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Glyph
	{
		float2 pos;
		uint index;
		float4 color;
		float2& GetPos() { return pos; }
		uint& GetIndex() { return index; }
		float4& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pos);
			compiler.compile(index);
			compiler.compile(color);
		}
	};
	#pragma pack(pop)
}