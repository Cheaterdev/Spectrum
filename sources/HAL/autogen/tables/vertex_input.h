#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct vertex_input
	{
		float2 pos;
		float2 tc;
		float4 mulColor;
		float4 addColor;
		float2& GetPos() { return pos; }
		float2& GetTc() { return tc; }
		float4& GetMulColor() { return mulColor; }
		float4& GetAddColor() { return addColor; }
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
