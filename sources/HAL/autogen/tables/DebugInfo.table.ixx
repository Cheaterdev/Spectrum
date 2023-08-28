export module HAL:Autogen.Tables.DebugInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.DebugStruct;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DebugInfo
	{
		static constexpr SlotID ID = SlotID::DebugInfo;
		HLSL::RWStructuredBuffer<DebugStruct> debug;
		HLSL::RWStructuredBuffer<DebugStruct>& GetDebug() { return debug; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(debug);
		}
		struct Compiled
		{
			uint debug; // RWStructuredBuffer<DebugStruct>
		};
	};
	#pragma pack(pop)
}
