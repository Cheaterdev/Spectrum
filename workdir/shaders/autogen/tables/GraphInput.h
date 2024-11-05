#pragma once
#include "sig_hlsl.hlsl"
struct GraphInput
{
	uint3 dispatch_grid: SV_DispatchGrid; // uint3
	uint unused; // uint
	int2 WaveOffset; // int2
	uint2 unused2; // uint2
	uint3 GetDispatch_grid() { return dispatch_grid; }
	uint GetUnused() { return unused; }
	int2 GetWaveOffset() { return WaveOffset; }
	uint2 GetUnused2() { return unused2; }
};