#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct ColorRect
	{
		float4 pos[2];
		float4 color;
		float4* GetPos() { return pos; }
		float4& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pos);
			compiler.compile(color);
		}
	};
	#pragma pack(pop)
}
