export module HAL:Autogen.Tables.LineRender;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.VSLine;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct LineRender
	{
		static constexpr SlotID ID = SlotID::LineRender;
		HLSL::StructuredBuffer<VSLine> vb;
		HLSL::StructuredBuffer<VSLine>& GetVb() { return vb; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vb);
		}
		struct Compiled
		{
			uint vb; // StructuredBuffer<VSLine>
		};
	};
	#pragma pack(pop)
}
