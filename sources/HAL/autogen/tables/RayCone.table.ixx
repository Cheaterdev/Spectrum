export module HAL:Autogen.Tables.RayCone;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct RayCone
	{
		static constexpr SlotID ID = SlotID::RayCone;
		float width;
		float angle;
		float& GetWidth() { return width; }
		float& GetAngle() { return angle; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(width);
			compiler.compile(angle);
		}
		using Compiled = RayCone;
		};
		#pragma pack(pop)
	}
