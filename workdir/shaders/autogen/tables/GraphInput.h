#pragma once
#include "sig_hlsl.hlsl"
struct GraphInput
{
	uint3 dispatch_grid: SV_DispatchGrid; // uint3
	uint3 GetDispatch_grid() { return dispatch_grid; }
};