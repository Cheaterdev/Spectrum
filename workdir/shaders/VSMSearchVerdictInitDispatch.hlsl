#include "autogen/VSMSearchVerdictInitDispatch.h"

// Turns VSM_BlockerSearch's own two post-search append counters
// (confirmed_lit/blur) into indirect dispatch arguments -- same shape as
// VSMBlockerClassifyInitDispatch, a separate instance because this append
// happens in a different pass's render() (see VSMSearchVerdictAppend's own
// comment in vsm.sig).
static const StructuredBuffer<uint> confirmed_lit_counter = GetVSMSearchVerdictInitDispatch().GetConfirmed_lit_counter();
static const StructuredBuffer<uint> blur_counter           = GetVSMSearchVerdictInitDispatch().GetBlur_counter();

static const RWStructuredBuffer<DispatchArguments> confirmed_lit_dispatch = GetVSMSearchVerdictInitDispatch().GetConfirmed_lit_dispatch_data();
static const RWStructuredBuffer<DispatchArguments> blur_dispatch          = GetVSMSearchVerdictInitDispatch().GetBlur_dispatch_data();

[numthreads(1, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	{
		DispatchArguments dinfo;
		dinfo.counts = uint3(confirmed_lit_counter[0], 1, 1);
		confirmed_lit_dispatch[0] = dinfo;
	}
	{
		DispatchArguments dinfo;
		dinfo.counts = uint3(blur_counter[0], 1, 1);
		blur_dispatch[0] = dinfo;
	}
}
