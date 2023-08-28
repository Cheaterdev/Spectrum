export module HAL:Autogen.Tables.Color;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Color
	{
		static constexpr SlotID ID = SlotID::Color;
		float4 color;
		float4& GetColor() { return color; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
		}
		using Compiled = Color;
		};
		#pragma pack(pop)
	}
