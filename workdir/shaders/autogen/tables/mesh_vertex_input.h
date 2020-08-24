#pragma once
struct mesh_vertex_input_cb
{
	float3 pos;
	float3 normal;
	float2 tc;
	float4 tangent;
};
struct mesh_vertex_input
{
	mesh_vertex_input_cb cb;
	float3 GetPos() { return cb.pos; }
	float3 GetNormal() { return cb.normal; }
	float2 GetTc() { return cb.tc; }
	float4 GetTangent() { return cb.tangent; }

};
 const mesh_vertex_input Createmesh_vertex_input(mesh_vertex_input_cb cb)
{
	const mesh_vertex_input result = {cb
	};
	return result;
}
