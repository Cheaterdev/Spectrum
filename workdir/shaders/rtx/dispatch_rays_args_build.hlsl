#include "../autogen/DispatchRaysArgsBuildData.h"

// Packs one D3D12_DISPATCH_RAYS_DESC-shaped record (DispatchRaysArguments,
// SIG.ixx's hand-written mirror of that struct) for a later ExecuteIndirect
// with D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS. Shader-table
// addresses/sizes/strides are constant for the RTXPSO's lifetime; width/
// height are this frame's caller-supplied dispatch size (see this PSO's
// own .prism comment). One thread -- 104 bytes of bookkeeping, not a workload.
[numthreads(1, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const DispatchRaysArgsBuildData data = GetDispatchRaysArgsBuildData();

	// DispatchRaysArguments is a plain [shader_only] struct, not a [Bind]
	// table -- its Get*() accessors return by value, not by reference (see
	// the generated tables/DispatchRaysArguments.h), so it's built via its
	// raw fields directly rather than through those accessors.
	DispatchRaysArguments args;
	args.raygen_addr     = data.GetRaygen_addr();
	args.raygen_size     = uint2(data.GetRaygen_size(), 0);
	args.miss_addr       = data.GetMiss_addr();
	args.miss_size       = uint2(data.GetMiss_stride() * data.GetMiss_count(), 0);
	args.miss_stride     = uint2(data.GetMiss_stride(), 0);
	args.hit_addr        = data.GetHit_addr();
	args.hit_size        = uint2(data.GetHit_stride() * data.GetHit_count(), 0);
	args.hit_stride      = uint2(data.GetHit_stride(), 0);
	args.callable_addr   = uint2(0, 0);
	args.callable_size   = uint2(0, 0);
	args.callable_stride = uint2(0, 0);
	args.width  = data.GetCompacted_count()[data.GetCount_index()] * data.GetWidth_multiplier();
	args.height = 1;
	args.depth  = 1;
	// Never read -- exists purely so this struct's HLSL stride (104 bytes)
	// matches what D3D12 requires for a DISPATCH_RAYS command signature's
	// ByteStride, and what the C++-side mirror's own natural alignment
	// already gives it (see this struct's own comment, raytracing.prism).
	args._pad = 0;

	data.GetArgs()[data.GetDest_index()] = args;
}
