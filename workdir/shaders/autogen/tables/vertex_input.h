#pragma once
struct vertex_input_cb
{
	float2 pos;
	float2 tc;
};
struct vertex_input
{
	vertex_input_cb cb;
	float2 GetPos() { return cb.pos; }
	float2 GetTc() { return cb.tc; }
};
 const vertex_input Createvertex_input(vertex_input_cb cb)
{
	const vertex_input result = {cb
	};
	return result;
}
