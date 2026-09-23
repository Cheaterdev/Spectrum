// v1 scaffold (see [[project-ddgi]] planning notes): stamps every probe's
// last_full_update_frame to 0 -- not a real round-robin budget yet (DDGI.ixx's
// Variable<int> probes_per_frame_budget, deferred, see plan). This exists to
// prove the SIG declarations/FrameGraph wiring compile and run cleanly, and
// to actually touch DDGI_Probes so PSO creation doesn't hit the engine's
// unused-bound-resource assert (HAL.D3D12.PipelineState.cpp) the way a fully
// empty shader body did.
#include "../autogen/DDGIProbeSelectData.h"

[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint4 probe_counts = GetDDGIProbeSelectData().GetInfo().GetProbe_counts();
	uint probe_count = probe_counts.x * probe_counts.y * probe_counts.z;

	uint linear_index = dispatchID.x;
	if (linear_index >= probe_count)
		return;

	// This cascade's own linear offset into the shared (DDGI_CascadeCount-
	// times-larger) probe buffer -- see DDGIInfo's own comment (ddgi.prism).
	uint probe_offset = GetDDGIProbeSelectData().GetInfo().GetCascade_info().x;
	GetDDGIProbeSelectData().GetProbes().GetProbes()[probe_offset + linear_index].last_full_update_frame = 0;
}
