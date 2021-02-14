#pragma once
struct vertex_input_cb
{
	float2 pos;
	float2 tc;
	float4 mulColor;
	float4 addColor;
};
struct vertex_input
{
	vertex_input_cb cb;
	float2 GetPos() { return cb.pos; }
	float2 GetTc() { return cb.tc; }
	float4 GetMulColor() { return cb.mulColor; }
	float4 GetAddColor() { return cb.addColor; }

};
 const vertex_input Createvertex_input(vertex_input_cb cb)
{
	const vertex_input result = {cb
	};
	return result;
}
