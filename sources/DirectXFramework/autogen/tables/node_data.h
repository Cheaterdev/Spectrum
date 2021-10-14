#pragma once
#include "AABB.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct node_data
	{
		struct CB
		{
			float4x4 node_global_matrix;
			float4x4 node_global_matrix_prev;
			float4x4 node_inverse_matrix;
			AABB::CB aabb;
		} &cb;
		float4x4& GetNode_global_matrix() { return cb.node_global_matrix; }
		float4x4& GetNode_global_matrix_prev() { return cb.node_global_matrix_prev; }
		float4x4& GetNode_inverse_matrix() { return cb.node_inverse_matrix; }
		AABB MapAabb() { return AABB(cb.aabb); }
		node_data(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
