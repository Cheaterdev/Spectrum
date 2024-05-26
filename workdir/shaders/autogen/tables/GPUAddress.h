#pragma once
#include "sig_hlsl.hlsl"
struct GPUAddress
{
	uint2 data; // uint2
	uint2 GetData() { return data; }
};
