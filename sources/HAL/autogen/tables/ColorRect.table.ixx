export module HAL:Autogen.Tables.ColorRect;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct ColorRect
	{
		static constexpr SlotID ID = SlotID::ColorRect;
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
		using Compiled = ColorRect;
		};
		#pragma pack(pop)
	}
