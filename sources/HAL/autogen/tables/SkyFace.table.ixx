export module HAL:Autogen.Tables.SkyFace;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SkyFace
	{
		static constexpr SlotID ID = SlotID::SkyFace;
		uint face;
		uint& GetFace() { return face; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(face);
		}
		using Compiled = SkyFace;
		};
		#pragma pack(pop)
	}
