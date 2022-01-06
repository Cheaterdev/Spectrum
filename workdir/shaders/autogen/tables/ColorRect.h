#pragma once
struct ColorRect
{
	float4 pos[2]; // float4
	float4 color; // float4
	float4 GetPos(int i) { return pos[i]; }
	float4 GetColor() { return color; }
};
