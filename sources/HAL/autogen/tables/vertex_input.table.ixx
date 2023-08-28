export module HAL:Autogen.Tables.vertex_input;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct vertex_input
	{
		static constexpr SlotID ID = SlotID::vertex_input;
		float2 pos;
		float2 tc;
		float4 mulColor;
		float4 addColor;
		float2& GetPos() { return pos; }
		float2& GetTc() { return tc; }
		float4& GetMulColor() { return mulColor; }
		float4& GetAddColor() { return addColor; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pos);
			compiler.compile(tc);
			compiler.compile(mulColor);
			compiler.compile(addColor);
		}
		using Compiled = vertex_input;
		};
		#pragma pack(pop)
	}
