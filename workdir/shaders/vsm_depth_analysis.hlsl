#include "Common.hlsl"

#include "autogen/FrameInfo.h"
#include "autogen/VSMDepthAnalysis.h"

static const GBuffer gbuffer = GetVSMDepthAnalysis().GetGbuffer();

groupshared uint group_min_bits;

// Central 50% of the screen only -- this is about what's near/important to
// the camera right now, not the whole view; a detailed spot near screen
// center shouldn't get diluted by scanning background at the edges.
// MIN reduction, not average, into the bit pattern of a positive float
// (InterlockedMin on the uint reinterpretation works because IEEE754
// preserves ordering for positive values) -- the single most-demanding
// point drives the result, not a blurred mean.
[numthreads(8, 8, 1)]
void CS(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID)
{
	if (all(gtid.xy == uint2(0, 0)))
		group_min_bits = asuint(3.402823466e+38); // FLT_MAX
	GroupMemoryBarrierWithGroupSync();

	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);

	uint2 region_min  = dims / 4;
	uint2 region_size = dims / 2;

	if (all(dtid.xy < region_size))
	{
		uint2 texel = region_min + dtid.xy;
		float2 tc = (float2(texel) + 0.5) / float2(dims);
		float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);

		if (raw_z > 0) // 0 = sky/no geometry, reversed-Z
		{
			Camera camera = GetFrameInfo().GetCamera();
			float3 wpos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());
			float dist = length(wpos - camera.GetPosition());

			// tan(fov_y/2) = 1/proj._22 for a symmetric perspective
			// projection (same trick used for the main camera's Hi-Z
			// occlusion screen-space radius estimate).
			float tan_half_fov = 1.0 / camera.GetProj()._22;
			float world_texel_size = 2.0 * dist * tan_half_fov / float(dims.y);

			InterlockedMin(group_min_bits, asuint(world_texel_size));
		}
	}

	GroupMemoryBarrierWithGroupSync();
	if (all(gtid.xy == uint2(0, 0)))
		InterlockedMin(GetVSMDepthAnalysis().GetResult()[0], group_min_bits);
}
