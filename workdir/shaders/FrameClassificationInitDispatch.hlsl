#include "autogen/FrameClassificationInitDispatch.h"

static const StructuredBuffer<uint> hi = GetFrameClassificationInitDispatch().GetHi_counter();
static const StructuredBuffer<uint> low = GetFrameClassificationInitDispatch().GetLow_counter();

static const RWStructuredBuffer<DispatchArguments> hi_init = GetFrameClassificationInitDispatch().GetHi_dispatch_data();
static const RWStructuredBuffer<DispatchArguments> low_init = GetFrameClassificationInitDispatch().GetLow_dispatch_data();

uint3 divide_by_multiple(uint3 v, uint3 a)
{
    return (v + a - 1) / a;
}

[numthreads(1, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{
    Log("bare call: ratio=%f hi=%u", (float)hi[0] / max(low[0], 1), hi[0]);

 //   LOG_TEST("via macro: hi*2=%u low*2=%u", hi[0] * 2, low[0] * 2);


    {
        DispatchArguments dinfo;
        dinfo.counts = divide_by_multiple(uint3(hi[0]*32,32,1), uint3(8, 8, 1));
        hi_init[0] = dinfo;
    }

	
    {
        DispatchArguments dinfo;
        dinfo.counts = divide_by_multiple(uint3(low[0] * 32, 32, 1), uint3(8, 8, 1));
        low_init[0] = dinfo;
    }

}
