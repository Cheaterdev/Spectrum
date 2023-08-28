export module HAL:Autogen.Tables.DrawStencil;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DrawStencil
	{
		static constexpr SlotID ID = SlotID::DrawStencil;
		HLSL::StructuredBuffer<float4> vertices;
		HLSL::StructuredBuffer<float4>& GetVertices() { return vertices; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertices);
		}
		struct Compiled
		{
			uint vertices; // StructuredBuffer<float4>
		};
	};
	#pragma pack(pop)
}
