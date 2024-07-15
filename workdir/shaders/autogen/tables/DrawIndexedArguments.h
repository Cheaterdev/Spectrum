#pragma once
#include "sig_hlsl.hlsl"
struct DrawIndexedArguments
{
	uint data[5]; // uint
	uint GetData(int i) { return data[i]; }
};