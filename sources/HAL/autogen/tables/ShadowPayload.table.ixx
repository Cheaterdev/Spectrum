export module HAL:Autogen.Tables.ShadowPayload;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct ShadowPayload
	{
		static constexpr SlotID ID = SlotID::ShadowPayload;
		bool hit;
		bool& GetHit() { return hit; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(hit);
		}
		using Compiled = ShadowPayload;
		};
		#pragma pack(pop)
	}
