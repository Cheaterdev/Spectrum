#pragma once
#include "mesh_vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct Triangle
	{
		float lod;
		mesh_vertex_input v;
		float& GetLod() { return lod; }
		mesh_vertex_input& GetV() { return v; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(lod);
			compiler.compile(v);
		}
		using Compiled = Triangle;
		};
		#pragma pack(pop)
	}
