#pragma once
#include "sig_hlsl.hlsl"
struct EnvFilter
{
	uint4 face; // uint4
	float4 scaler; // float4
	uint4 size; // uint4
	uint4 GetFace() { return face; }
	float4 GetScaler() { return scaler; }
	uint4 GetSize() { return size; }
};
