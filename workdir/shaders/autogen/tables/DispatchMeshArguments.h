#pragma once
#include "sig_hlsl.hlsl"
struct DispatchMeshArguments
{
	uint3 counts; // uint3
	uint3 GetCounts() { return counts; }
};