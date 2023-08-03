export module HAL:Autogen.Tables.TilingPostprocess;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.TilingParams;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct TilingPostprocess
	{
		static constexpr SlotID ID = SlotID::TilingPostprocess;
		TilingParams tiling;
		TilingParams& GetTiling() { return tiling; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tiling);
		}
		using Compiled = TilingPostprocess;
		};
		#pragma pack(pop)
	}
