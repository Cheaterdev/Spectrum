// Spike (see [[project-nrd-integration]]): second kernel moved onto the
// universal NRD_Universal struct/shim -- proves the shared REBLUR
// shared-constants shim (reblur_shared_constants_shim.hlsli) alongside the
// shared resource shim (nrd_universal_shim.hlsli), replacing this kernel's
// own hand-typed REBLUR_ClassifyTilesResources struct and its 77-line
// inline constants-forwarding block.
//
// NRD_INTERNAL must be defined before NRD.hlsli is reached -- it gates the
// RADIANCE/SH/OCCLUSION/DIFF/SPEC/NRD_DIFF/NRD_SPEC macros REBLUR's own
// resources.hlsli #if blocks depend on. NRD_SIGNAL=DIFF / NRD_MODE=RADIANCE
// match this instance's actual permutation (REBLUR_DIFFUSE, non-SH,
// non-occlusion) -- same permutation this kernel always compiled as.
#define NRD_INTERNAL
#define NRD_SIGNAL DIFF
#define NRD_MODE RADIANCE

#include "reblur_shared_constants_shim.hlsli"
#include "nrd_universal_shim.hlsli"

// TEMP debug: this file lives one level deeper (workdir/shaders/nrd/) than
// the auto-include's assumed "autogen/DebugInfo.h" (workdir/shaders/-relative)
// resolves from, so it 404s unless included explicitly with the right
// relative path here.
#include "../autogen/DebugInfo.h"

// TEMP debug (see [[project-nrd-integration]] investigation, 2026-09-09):
// rename the real entry point so we can wrap it and log what the universal
// struct actually delivered, without touching the vendored .cs.hlsl file.
#define NRD_CS_MAIN nrd_real_main
#include "3rdparty/REBLUR_ClassifyTiles.cs.hlsl"
#undef NRD_CS_MAIN

[numthreads( 8, 4, 1 )]
void main( uint2 threadPos : SV_GroupThreadId, uint2 tilePos : SV_GroupId, uint threadIndex : SV_GroupIndex )
{
	if (tilePos.x == 0 && tilePos.y == 0 && threadIndex == 0)
	{
		Log("CT: denoisRange=%f worldToClip00=%f slotIn0=%u slotOut0=%u",
			asuint(gDenoisingRange), asuint(gWorldToClip[0][0]),
			GetNRD_Universal().GetSlotsIn(0), GetNRD_Universal().GetSlotsOut(0));
	}
	nrd_real_main(threadPos, tilePos, threadIndex);
}
