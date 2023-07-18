#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Color
	{
		static constexpr SlotID ID = SlotID::Color;
		float4 color;
		float4& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
		}
		using Compiled = Color;
		};
		#pragma pack(pop)
	}
