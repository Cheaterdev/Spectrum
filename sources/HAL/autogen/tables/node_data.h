#pragma once
#include "AABB.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct node_data
	{
		float4x4 node_global_matrix;
		float4x4 node_global_matrix_prev;
		float4x4 node_inverse_matrix;
		AABB aabb;
		float4x4& GetNode_global_matrix() { return node_global_matrix; }
		float4x4& GetNode_global_matrix_prev() { return node_global_matrix_prev; }
		float4x4& GetNode_inverse_matrix() { return node_inverse_matrix; }
		AABB& GetAabb() { return aabb; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(node_global_matrix);
			compiler.compile(node_global_matrix_prev);
			compiler.compile(node_inverse_matrix);
			compiler.compile(aabb);
		}
		using Compiled = node_data;
		};
	#pragma pack(pop)
}
