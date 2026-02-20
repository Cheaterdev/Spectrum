export module HAL:Autogen.Tables.AABB;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
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
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(min);
			compiler.compile(max);
		}
		using Compiled = AABB;

		static std::string get_typename()
		{
			return "Tables::AABB";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(min);
			ar& NVP(max);
		}

	};
	#pragma pack(pop)
}

