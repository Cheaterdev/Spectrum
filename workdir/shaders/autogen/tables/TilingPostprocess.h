#pragma once
#include "TilingParams.h"
struct TilingPostprocess
{
	TilingParams tiling; // TilingParams
	TilingParams GetTiling() { return tiling; }
};
