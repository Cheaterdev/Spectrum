#pragma once
#include "sig_hlsl.hlsl"
struct Instance
{
	uint instanceId; // uint
	uint GetInstanceId() { return instanceId; }
};