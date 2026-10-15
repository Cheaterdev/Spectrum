#include "../../common/common.hlsl"

#include "../../autogen/SkyAmbientData.h"
#include "../../autogen/FrameInfo.h"

static const GBuffer gbuffer = GetSkyAmbientData().GetGbuffer();

#include "../../common/pbr.hlsl"
#include "../../common/sky_indirect.hlsl"

// Adds the baked sky's indirect light onto the direct light already in the result (sky_indirect.hlsl).
[numthreads(16, 16, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	GetSkyAmbientData().GetResult().GetDimensions(dims.x, dims.y);
	if (any(DTid.xy >= dims))
		return;

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	if (raw_z == 0)
		return;

	Camera camera = GetFrameInfo().GetCamera();
	float4 packed_0 = gbuffer.GetAlbedo().SampleLevel(pointClampSampler, tc, 0);
	float4 packed_n = gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0);
	float3 normal = normalize(packed_n.xyz * 2 - 1);
	float roughness = pow(max(MIN_ROUGHNESS, packed_n.w), 2);
	float3 pos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());
	float3 view = normalize(camera.GetPosition() - pos);

	float4 direct = GetSkyAmbientData().GetResult()[DTid.xy];
	float3 indirect = sky_indirect_light(GetSkyAmbientData().GetSky(), packed_0.rgb, packed_0.w, normal, view, roughness);
	GetSkyAmbientData().GetResult()[DTid.xy] = float4(direct.rgb + indirect, direct.a);
}
