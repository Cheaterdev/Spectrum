export module HAL:Autogen.Tables.FontRenderingGlyphs;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.Glyph;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingGlyphs
	{
		static constexpr SlotID ID = SlotID::FontRenderingGlyphs;
		HLSL::StructuredBuffer<Glyph> data;
		HLSL::StructuredBuffer<Glyph>& GetData() { return data; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
		}
		struct Compiled
		{
			uint data; // StructuredBuffer<Glyph>
		};
	};
	#pragma pack(pop)
}
