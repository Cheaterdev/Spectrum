#include "../autogen/DDGIProbeResidencyMarkData.h"

// v1 placeholder (see [[project-ddgi]] planning notes): marks every probe
// needed unconditionally, matching today's "trace everything" behavior
// exactly. Proves the residency buffer's create/read/write plumbing
// (DDGIProbeSelect creates it, this pass writes it, DDGIProbeTrace/Convolve
// read it) is correct with zero behavior change before the real
// hit-point-driven marking + neighbor dilation replaces this body.
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const DDGIProbeResidencyMarkData data = GetDDGIProbeResidencyMarkData();

	uint4 probe_counts = data.GetInfo().GetProbe_counts();
	uint probe_count = probe_counts.x * probe_counts.y * probe_counts.z;

	uint linear_index = dispatchID.x;
	if (linear_index >= probe_count)
		return;

	uint probe_offset = data.GetInfo().GetCascade_info().x;
	data.GetProbe_residency()[probe_offset + linear_index] = 1;
}
