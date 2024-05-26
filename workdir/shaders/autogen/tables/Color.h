#pragma once
#include "sig_hlsl.hlsl"
struct Color
{
	float4 color; // float4
	float4 GetColor() { return color; }
};
