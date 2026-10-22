#include "../../common/common.hlsl"

#include "../../autogen/SkyAmbientData.h"
#include "../../autogen/FrameInfo.h"

static const GBuffer gbuffer = GetSkyAmbientData().GetGbuffer();

#include "../../common/pbr.hlsl"
#include "../../common/sky_indirect.hlsl"

static const uint SKY_DIFFUSE  = 1;
static const uint SKY_SPECULAR = 2;

// Adds the baked sky's indirect light onto the direct light already in the result (sky_indirect.hlsl).
void add_sky(uint2 pixel, uint parts)
{
	uint2 dims;
	GetSkyAmbientData().GetResult().GetDimensions(dims.x, dims.y);
	if (any(pixel >= dims))
		return;

	float2 tc = (float2(pixel) + 0.5) / float2(dims);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	if (raw_z == 0)
		return;

	Camera camera = GetFrameInfo().GetCamera();
	float4 packed_0 = gbuffer.GetAlbedo().SampleLevel(pointClampSampler, tc, 0);
	float4 packed_n = gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0);
	float4 material = gbuffer.GetMaterial().SampleLevel(pointClampSampler, tc, 0);
	float3 normal = normalize(packed_n.xyz * 2 - 1);
	float roughness = pow(max(MIN_ROUGHNESS, packed_n.w), 2);
	float3 pos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());
	float3 view = normalize(camera.GetPosition() - pos);

	TextureCube<float4> sky = GetSkyAmbientData().GetSky();
	TextureCube<float4> sky_diffuse = GetSkyAmbientData().GetSky_diffuse();
	float3 indirect = 0;

	if (parts & SKY_DIFFUSE)
		indirect += sky_diffuse_light(sky_diffuse, material, packed_0.rgb, packed_0.w, normal, view);

	if (parts & SKY_SPECULAR)
		indirect += sky_specular_light(sky, material, packed_0.rgb, packed_0.w, normal, view, roughness);

	float4 direct = GetSkyAmbientData().GetResult()[pixel];
	GetSkyAmbientData().GetResult()[pixel] = float4(direct.rgb + indirect, direct.a);
}

[numthreads(16, 16, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	add_sky(DTid.xy, SKY_DIFFUSE | SKY_SPECULAR);
}

// The two halves on their own, for a pipeline that blurs the diffuse light (subsurface
// scattering) between them.
[numthreads(16, 16, 1)]
void CS_DIFFUSE(uint3 DTid : SV_DispatchThreadID)
{
	add_sky(DTid.xy, SKY_DIFFUSE);
}

[numthreads(16, 16, 1)]
void CS_SPECULAR(uint3 DTid : SV_DispatchThreadID)
{
	add_sky(DTid.xy, SKY_SPECULAR);
}
