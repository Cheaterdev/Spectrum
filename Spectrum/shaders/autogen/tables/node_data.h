#pragma once
struct node_data_cb
{
	float4x4 node_global_matrix;
	float4x4 node_inverse_matrix;
};
struct node_data
{
	node_data_cb cb;
	float4x4 GetNode_global_matrix() { return cb.node_global_matrix; }
	float4x4 GetNode_inverse_matrix() { return cb.node_inverse_matrix; }
};
 const node_data Createnode_data(node_data_cb cb)
{
	const node_data result = {cb
	};
	return result;
}
