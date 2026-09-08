// Debug-only unpack of REBLUR's packed output (YCoCg + normalized hit
// distance) into plain RGB, for the "RTX Indirect (REBLUR), Unpacked" debug
// view (see [[project-nrd-integration]], nrd_sig_test.sig's comment). Not
// used by the real consumer (RTXCombine unpacks inline in rtx_combine.hlsl).
#include "../autogen/NRD_UnpackDebugParams.h"
#include "3rdparty/NRD.hlsli"

static const Texture2D<float4>   Packed   = GetNRD_UnpackDebugParams().GetPacked();
static const RWTexture2D<float4> Unpacked = GetNRD_UnpackDebugParams().GetUnpacked();

[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 size;
	Unpacked.GetDimensions(size.x, size.y);
	if (any(dispatchID.xy >= size)) return;

	Unpacked[dispatchID.xy] = REBLUR_BackEnd_UnpackRadianceAndNormHitDist(Packed[dispatchID.xy]);
}
