#pragma once
struct VSLine
{
	float2 pos; // float2
	float4 color; // float4
	float2 GetPos() { return pos; }
	float4 GetColor() { return color; }
};
