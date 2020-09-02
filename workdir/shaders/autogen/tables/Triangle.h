#pragma once
#include "mesh_vertex_input.h"
struct Triangle
{
	float lod;
	mesh_vertex_input v;

		void init(mesh_vertex_input vertex0, mesh_vertex_input vertex1, mesh_vertex_input vertex2, float3 barycentrics)
		{
			v.normal = (vertex0.normal * barycentrics.x + vertex1.normal * barycentrics.y + vertex2.normal * barycentrics.z);
			v.tc = vertex0.tc * barycentrics.x + vertex1.tc * barycentrics.y + vertex2.tc * barycentrics.z;

			float P_a = length(cross(vertex2.pos - vertex0.pos, vertex1.pos - vertex0.pos));
			float T_a =  length(cross(float3(vertex2.tc - vertex0.tc,0), float3(vertex1.tc - vertex0.tc,0 )));
			lod = 0.5 * log2(T_a / P_a);

		}
	
};
