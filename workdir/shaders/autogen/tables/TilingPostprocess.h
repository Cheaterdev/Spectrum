#pragma once
#include "sig_hlsl.hlsl"
#include "TilingParams.h"
struct TilingPostprocess
{
	TilingParams tiling; // TilingParams
	TilingParams GetTiling() { return tiling; }
};