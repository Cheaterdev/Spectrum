export module HAL:Autogen.Tables.Glyph;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
export namespace Table
{
	#pragma pack(push, 1)

	struct Glyph
	{
		static constexpr SlotID ID = SlotID::Glyph;
		float2 pos;
		uint index;
		float4 color;
		float2& GetPos() { return pos; }
		uint& GetIndex() { return index; }
		float4& GetColor() { return color; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pos);
			compiler.compile(index);
			compiler.compile(color);
		}
		using Compiled = Glyph;


		private:
		SERIALIZE()
		{
			ar& NVP(pos);
			ar& NVP(index);
			ar& NVP(color);
		}

	};
	#pragma pack(pop)
}

