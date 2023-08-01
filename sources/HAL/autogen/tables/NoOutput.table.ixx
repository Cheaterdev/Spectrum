export module HAL:Autogen.Tables.NoOutput;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct NoOutput
	{
		static constexpr SlotID ID = SlotID::NoOutput;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
		}
		using Compiled = NoOutput;
		};
		#pragma pack(pop)
	}
