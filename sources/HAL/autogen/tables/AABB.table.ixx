export module HAL:Autogen.Tables.AABB;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct AABB
	{
		static constexpr SlotID ID = SlotID::AABB;
		float4 min;
		float4 max;
		float4& GetMin() { return min; }
		float4& GetMax() { return max; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(min);
			compiler.compile(max);
		}
		using Compiled = AABB;
		};
		#pragma pack(pop)
	}
