export module HAL:Autogen.Tables.DebugStruct;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DebugStruct
	{
		static constexpr SlotID ID = SlotID::DebugStruct;
		uint4 v;
		uint4& GetV() { return v; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(v);
		}
		using Compiled = DebugStruct;
		};
		#pragma pack(pop)
	}
