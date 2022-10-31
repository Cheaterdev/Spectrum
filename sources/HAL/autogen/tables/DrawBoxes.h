#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawBoxes
	{
		HLSL::StructuredBuffer<float4> vertices;
		HLSL::StructuredBuffer<BoxInfo> input_meshes;
		HLSL::RWStructuredBuffer<uint> visible_meshes;
		HLSL::StructuredBuffer<float4>& GetVertices() { return vertices; }
		HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return input_meshes; }
		HLSL::RWStructuredBuffer<uint>& GetVisible_meshes() { return visible_meshes; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertices);
			compiler.compile(input_meshes);
			compiler.compile(visible_meshes);
		}
	};
	#pragma pack(pop)
}