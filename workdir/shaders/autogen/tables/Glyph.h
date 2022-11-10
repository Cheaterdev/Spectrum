#pragma once
struct Glyph
{
	float2 pos; // float2
	uint index; // uint
	float4 color; // float4
	float2 GetPos() { return pos; }
	uint GetIndex() { return index; }
	float4 GetColor() { return color; }
};
