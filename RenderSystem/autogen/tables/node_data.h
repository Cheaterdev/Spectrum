#pragma once
namespace Table 
{
	struct node_data
	{
		struct CB
		{
			float4x4 node_global_matrix;
			float4x4 node_inverse_matrix;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4x4& GetNode_global_matrix() { return cb.node_global_matrix; }
		float4x4& GetNode_inverse_matrix() { return cb.node_inverse_matrix; }
		node_data(CB&cb) :cb(cb){}
	};
}
