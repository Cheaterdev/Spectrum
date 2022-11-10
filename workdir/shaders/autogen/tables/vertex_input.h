#pragma once
struct vertex_input
{
	float2 pos; // float2
	float2 tc; // float2
	float4 mulColor; // float4
	float4 addColor; // float4
	float2 GetPos() { return pos; }
	float2 GetTc() { return tc; }
	float4 GetMulColor() { return mulColor; }
	float4 GetAddColor() { return addColor; }
};
