// v1 scaffold (see [[project-ddgi]] planning notes): clears this probe's
// radiance/gbuffer texel instead of actually tracing -- no TraceRay call yet
// (real per-probe sphere sampling + MyClosestHitShader reuse is a later plan
// step). Exists to prove the SIG declarations, RTPSO binding and FrameGraph
// wiring compile and run cleanly, and to actually touch its bound resources
// so PSO/state-object creation doesn't hit the engine's unused-bound-
// resource assert the way a fully empty raygen body did.
#include "../autogen/DDGIProbeTraceData.h"

[shader("raygeneration")]
void DDGIProbeTraceRaygenShader()
{
	// Raygen shaders use CreateX()+a local, not the GetX() global convenience
	// wrapper -- same pattern every raygen in raytracing.hlsl uses
	// (CreateVoxelOutput()/CreateVoxelScreen()); GetX() is guarded out for
	// raygen compilation (NO_GLOBAL), confirmed the hard way.
	const DDGIProbeTraceData trace_data = CreateDDGIProbeTraceData();

	uint3 launch_index = DispatchRaysIndex();

	trace_data.GetProbe_radiance()[launch_index.xy] = 0;
	trace_data.GetProbe_gbuffer()[launch_index.xy] = 0;
}
