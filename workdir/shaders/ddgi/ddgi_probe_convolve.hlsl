// v1 scaffold (see [[project-ddgi]] planning notes): copies the raw radiance
// texel straight through instead of actually convolving -- real cosine
// convolution / chebyshev visibility filtering is a later plan step. Exists
// to prove the SIG declarations and FrameGraph wiring compile and run
// cleanly, and to actually touch its bound resources so PSO creation doesn't
// hit the engine's unused-bound-resource assert the way a fully empty
// shader body did.
#include "../autogen/DDGIProbeConvolveData.h"

[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	float4 radiance = GetDDGIProbeConvolveData().GetProbe_radiance()[dispatchID.xy];

	GetDDGIProbeConvolveData().GetProbe_irradiance()[dispatchID.xy] = radiance;
	GetDDGIProbeConvolveData().GetProbe_visibility()[dispatchID.xy] = 0;
}
