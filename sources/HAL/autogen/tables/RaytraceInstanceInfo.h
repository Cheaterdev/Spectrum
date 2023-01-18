#pragma once
#include "mesh_vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct RaytraceInstanceInfo
	{
		HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
		HLSL::StructuredBuffer<uint> indices;
		HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return vertexes; }
		HLSL::StructuredBuffer<uint>& GetIndices() { return indices; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertexes);
			compiler.compile(indices);
		}
		struct Compiled
		{
			uint vertexes; // StructuredBuffer<mesh_vertex_input>
			uint indices; // StructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
