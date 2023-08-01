export module HAL:Autogen.Tables.VSLine;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct VSLine
	{
		static constexpr SlotID ID = SlotID::VSLine;
		float2 pos;
		float4 color;
		float2& GetPos() { return pos; }
		float4& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pos);
			compiler.compile(color);
		}
		using Compiled = VSLine;
		};
		#pragma pack(pop)
	}
