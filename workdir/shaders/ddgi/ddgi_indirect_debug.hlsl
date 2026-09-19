#include "../autogen/FrameInfo.h"
#include "../autogen/DDGIIndirectDebugData.h"
#include "../common/common.hlsl"
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

	float3 indirect = ddgi_sample_irradiance_cascaded(pos, normal,
		data.GetCascade0(), data.GetCascade1(), data.GetCascade2(), data.GetCascade3(), data.GetCascade4(),
		data.GetProbe_irradiance(), data.GetProbe_visibility());

	data.GetTarget()[itc] = float4(indirect, 1);
}
