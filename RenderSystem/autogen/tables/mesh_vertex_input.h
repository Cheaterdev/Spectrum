#pragma once
namespace Table 
{
	struct mesh_vertex_input
	{
		struct CB
		{
			float3 pos;
			float3 normal;
			float2 tc;
			float4 tangent;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float3& GetPos() { return cb.pos; }
		float3& GetNormal() { return cb.normal; }
		float2& GetTc() { return cb.tc; }
		float4& GetTangent() { return cb.tangent; }
		mesh_vertex_input(CB&cb) :cb(cb){}
	};
}
