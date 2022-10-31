#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Color
	{
		float4 color;
		float4& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
		}
	};
	#pragma pack(pop)
}
