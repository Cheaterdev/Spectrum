#pragma once
#include "mesh_vertex_input.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct Triangle
	{
		struct CB
		{
			float lod;
			mesh_vertex_input::CB v;
		} &cb;
		float& GetLod() { return cb.lod; }
		mesh_vertex_input MapV() { return mesh_vertex_input(cb.v); }
		Triangle(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
