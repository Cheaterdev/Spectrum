#pragma once
#include "sig_hlsl.hlsl"
struct AABB
{
	float4 min; // float4
	float4 max; // float4
	float4 GetMin() { return min; }
	float4 GetMax() { return max; }
};
