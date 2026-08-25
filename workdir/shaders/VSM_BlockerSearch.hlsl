#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMConstants.h"
#include "autogen/FrameInfo.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

#include "VSM_impl.hlsl"

// Compute-queue: one thread per output pixel, writes the blocker-search
// result UAV (VSMLighting's blocker_result field) for VSM_Combine's resolve
// step to read back. Same size as VSM_Combine's own dispatch -- see vsm.sig's
// VSM_BlockerSearch PassNode comment for why this is a separate dispatch now
// instead of running inline inside VSM_Combine's own shader.
//
// Mirrors VSM.hlsl's combine_result/CS_RESULT split for the SAME reason:
// vsm_search_blocker's quad-shared search mode (VSM_impl.hlsl,
// quad_blocker_search == 1) needs every thread in a 2x2 dispatch quad to
// reach its QuadReadAcrossX/Y/Diagonal calls uniformly -- a compute-shader
// `return` genuinely deactivates a thread (no pixel-shader helper-invocation
// guarantee), so an early return for sky pixels or screen-edge padding
// threads would leave still-active quad-mates' reads undefined. This pass
// had exactly that bug class before the fix (and later the search itself)
// moved here -- see VSM.hlsl's own combine_result/CS_RESULT comments, which
// now describe the SIMPLER form this pass used to need before the
// extraction, since they no longer do any quad ops themselves.
[numthreads(16, 16, 1)]
void CS_BLOCKER_SEARCH(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	GetVSMLighting().GetBlocker_result().GetDimensions(dims.x, dims.y);

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	bool has_geometry = (raw_z != 0);

	Camera camera = GetFrameInfo().GetCamera();
	// Sky pixels get safe, finite dummy inputs (camera position / a fixed
	// up vector) rather than whatever depth_to_wpos(0, ...) or an unwritten
	// gbuffer normal texel would produce -- vsm_search_blocker never sees
	// NaN/Inf-risking input even though its result is discarded below.
	float3 wpos = has_geometry ? depth_to_wpos(raw_z, tc, camera.GetInvViewProj()) : camera.GetPosition();

	VSMConstants constants = GetVSMConstants();
	uint4 result = vsm_search_blocker(constants, GetVSMLighting(), wpos, DTid.xy);
	if (!has_geometry)
		result = uint4(asuint(-1.0), 0, 0, 0); // sentinel: no blocker (matches vsm_search_blocker's own !valid convention)

	if (all(DTid.xy < dims))
		GetVSMLighting().GetBlocker_result()[DTid.xy] = result;
}
