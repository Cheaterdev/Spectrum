#pragma once
#include "sig_hlsl.hlsl"
struct DispatchArguments
{
	uint3 counts; // uint3
	uint3 GetCounts() { return counts; }
};
