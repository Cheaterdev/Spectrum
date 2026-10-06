#include "../autogen/FGTRayQuery.h"

// One ray per texel, from z = +1 straight down over [-1,1]^2; writes the
// committed hit distance, 0 on a miss (Test.RTX.ixx, RayQuery_InstancedTriangles).
[numthreads(8, 8, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
	const uint2 size = GetFGTRayQuery().GetSize();
	if (any(id.xy >= size))
		return;

	const float2 p = (float2(id.xy) + 0.5) / float2(size) * 2 - 1;

	RayDesc ray;
	ray.Origin    = float3(p.x, -p.y, 1);
	ray.Direction = float3(0, 0, -1);
	ray.TMin      = 0;
	ray.TMax      = 10;

	RayQuery<RAY_FLAG_FORCE_OPAQUE> q;
	q.TraceRayInline(GetFGTRayQuery().GetScene(), RAY_FLAG_NONE, 0xFF, ray);
	q.Proceed();

	GetFGTRayQuery().GetOutput()[id.xy] = q.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? q.CommittedRayT() : 0;
}
