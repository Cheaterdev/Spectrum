#pragma once
#include "sig_hlsl.hlsl"
struct PSSMConstants
{
	int level; // int
	float time; // float
	int GetLevel() { return level; }
	float GetTime() { return time; }
};