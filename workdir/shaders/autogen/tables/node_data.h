#pragma once
#include "AABB.h"
struct node_data_cb
{
	float4x4 node_global_matrix;
	float4x4 node_inverse_matrix;
	AABB_cb aabb;
};
struct node_data
{
	node_data_cb cb;
	float4x4 GetNode_global_matrix() { return cb.node_global_matrix; }
	float4x4 GetNode_inverse_matrix() { return cb.node_inverse_matrix; }
	AABB GetAabb() { return CreateAABB(cb.aabb); }

};
 const node_data Createnode_data(node_data_cb cb)
{
	const node_data result = {cb
	};
	return result;
}
