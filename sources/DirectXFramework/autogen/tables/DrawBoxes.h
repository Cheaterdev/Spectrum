#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawBoxes
	{
		Render::HLSL::StructuredBuffer<float4> vertices;
		Render::HLSL::StructuredBuffer<BoxInfo> input_meshes;
		Render::HLSL::RWStructuredBuffer<uint> visible_meshes;
		Render::HLSL::StructuredBuffer<float4>& GetVertices() { return vertices; }
		Render::HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return input_meshes; }
		Render::HLSL::RWStructuredBuffer<uint>& GetVisible_meshes() { return visible_meshes; }
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
