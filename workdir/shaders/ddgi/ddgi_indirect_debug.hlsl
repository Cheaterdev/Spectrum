#include "../autogen/FrameInfo.h"
#include "../autogen/Raytracing.h"
#include "../autogen/DDGIIndirectDebugData.h"
#include "../common/common.hlsl"
// Real inline-ray-traced probe occlusion instead of the chebyshev heuristic
// -- see ddgi_sample.hlsl's own comment on ddgi_sample_irradiance_inline_traced.
// This is a plain compute shader (can't call classic TraceRay at all), which
// is exactly what the INLINE (RayQuery) variant is for.
#define DDGI_SAMPLE_ENABLE_INLINE_TRACED_VISIBILITY
#include "ddgi_sample.hlsl"

// Full-screen debug view (FrameGraph::DebugMode::DDGIIndirect, see
// Base.cpp's debug_source()) of the exact same ddgi_sample_irradiance()
// call IndirectRTX's own per-pixel feedback term uses (ddgi_sample.hlsl) --
// but shown in isolation, at every screen pixel, using each pixel's real
// GBuffer_Normals (not DDGIDebug's marker-only camera-facing hack), to
// judge the trilinear + chebyshev-visibility interpolation's quality
// directly instead of only at discrete probe positions. Raw irradiance, not
// albedo-modulated -- same convention as RTXIndirectDenoised showing the
// raw GI signal.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const DDGIIndirectDebugData data = GetDDGIIndirectDebugData();

	uint width, height;
	data.GetTarget().GetDimensions(width, height);

	uint2 itc = dispatchID.xy;
	if (itc.x >= width || itc.y >= height)
		return;

	float2 tc = (float2(itc) + 0.5) / float2(width, height);

	float raw_z = data.GetDepth()[itc];
	if (raw_z == 0)
	{
		data.GetTarget()[itc] = float4(0, 0, 0, 1);
		return;
	}

	const FrameInfo frame = GetFrameInfo();
	float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());
	float3 normal = normalize(data.GetNormals()[itc].xyz * 2 - 1);

	// Marks residency at this pixel's own surface point -- same 8-corner
	// scheme TraceIndirectDiffuse's own marking block uses (raytracing.hlsl),
	// just against `pos` (this view's own sample point) instead of a
	// secondary bounce hit point, since that's what this view actually reads
	// from. Lets this debug view stay accurate under real residency culling
	// (see the PassNode's own comment, ddgi.sig) instead of needing "Enable
	// residency culling" turned off globally just to inspect one area, which
	// traces/convolves every probe in the whole grid regardless of whether
	// this view is even pointed at it.
	{
		DDGIInfo ddgi_cascades[4] = {
			data.GetCascade0(), data.GetCascade1(), data.GetCascade2(), data.GetCascade3()
		};
		DDGIProbes probes;
		[unroll]
		for (int c = 0; c < 4; c++)
		{
			float3 local = (pos - ddgi_cascades[c].GetGrid_min().xyz) / ddgi_cascades[c].GetProbe_spacing().xyz;
			int3 base = int3(floor(local));
			int3 probe_counts = int3(ddgi_cascades[c].GetProbe_counts().xyz);
			uint cascade_offset = ddgi_cascades[c].GetCascade_info().x;

			[unroll]
			for (uint i = 0; i < 8; i++)
			{
				int3 corner = int3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
				int3 cell = base + corner;
				if (all(cell >= 0) && all(cell < probe_counts))
				{
					uint linear_index = probes.ddgi_probe_linear_index(uint3(cell), uint3(probe_counts));
					data.GetResidency_pending()[cascade_offset + linear_index] = 1;
				}
			}
		}
	}

	float3 indirect = ddgi_sample_irradiance_cascaded_inline_traced(pos, normal,
		data.GetCascade0(), data.GetCascade1(), data.GetCascade2(), data.GetCascade3(), data.GetCascade4(),
		data.GetProbe_irradiance(), data.GetProbe_visibility(), data.GetProbe_residency(),
		GetRaytracing().GetScene());

	data.GetTarget()[itc] = float4(indirect, 1);
}
