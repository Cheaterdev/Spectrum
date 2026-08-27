#include "autogen/VSMBlockerClassifyInitDispatch.h"

// Turns VSM_BlockerClassify's three GPU-written append counters
// (lit/dark/search) into indirect dispatch arguments, one shader
// invocation instead of three -- mirrors FrameClassificationInitDispatch
// exactly. One appended tile is exactly one 16x16 group in each consumer
// (tile size == group size here, unlike VoxelGI's 32x32-tile-via-8x8-groups
// shape), so each tile count copies straight into its own counts.x with no
// divide_by_multiple scaling needed.
static const StructuredBuffer<uint> lit_counter    = GetVSMBlockerClassifyInitDispatch().GetLit_counter();
static const StructuredBuffer<uint> dark_counter   = GetVSMBlockerClassifyInitDispatch().GetDark_counter();
static const StructuredBuffer<uint> search_counter = GetVSMBlockerClassifyInitDispatch().GetSearch_counter();

static const RWStructuredBuffer<DispatchArguments> lit_dispatch    = GetVSMBlockerClassifyInitDispatch().GetLit_dispatch_data();
static const RWStructuredBuffer<DispatchArguments> dark_dispatch   = GetVSMBlockerClassifyInitDispatch().GetDark_dispatch_data();
static const RWStructuredBuffer<DispatchArguments> search_dispatch = GetVSMBlockerClassifyInitDispatch().GetSearch_dispatch_data();

[numthreads(1, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	{
		DispatchArguments dinfo;
		dinfo.counts = uint3(lit_counter[0], 1, 1);
		lit_dispatch[0] = dinfo;
	}
	{
		DispatchArguments dinfo;
		dinfo.counts = uint3(dark_counter[0], 1, 1);
		dark_dispatch[0] = dinfo;
	}
	{
		DispatchArguments dinfo;
		dinfo.counts = uint3(search_counter[0], 1, 1);
		search_dispatch[0] = dinfo;
	}
}
