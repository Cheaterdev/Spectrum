#pragma once
#include "AABB.h"
struct node_data
{
	float4x4 node_global_matrix; // float4x4
	float4x4 node_global_matrix_prev; // float4x4
	float4x4 node_inverse_matrix; // float4x4
	AABB aabb; // AABB
	float4x4 GetNode_global_matrix() { return node_global_matrix; }
	float4x4 GetNode_global_matrix_prev() { return node_global_matrix_prev; }
	float4x4 GetNode_inverse_matrix() { return node_inverse_matrix; }
	AABB GetAabb() { return aabb; }
};
