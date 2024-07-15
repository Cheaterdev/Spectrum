#pragma once
#include "sig_hlsl.hlsl"
struct DebugStruct
{
	uint4 v; // uint4
	uint4 GetV() { return v; }
};