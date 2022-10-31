#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawStencil
	{
		HLSL::StructuredBuffer<float4> vertices;
		HLSL::StructuredBuffer<float4>& GetVertices() { return vertices; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertices);
		}
	};
	#pragma pack(pop)
}