// Needs FrameInfo.h (the BRDF lookup) and the common samplers included first.
// The baked sky as the indirect light of a pipeline with no GI (ray tracing, voxel GI): diffuse ambient from the
// blurred top mip of the specular-filtered cube, and the reflection a metal shows. Without it a metallic
// material (spec_to_metallic) has no diffuse term and renders black.
float3 sky_indirect_light(TextureCube<float4> sky, float3 albedo, float metallic, float3 normal, float3 view, float roughness)
{
	const float sky_mips = 6;   // sky_cubemap_filtered has 7
	float NV = saturate(dot(normal, view));
	float3 diffuse_color = albedo * (1 - metallic);
	float3 f0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
	float3 ambient = sky.SampleLevel(linearClampSampler, normal, sky_mips).rgb;
	float3 reflection = sky.SampleLevel(linearClampSampler, reflect(-view, normal), roughness * sky_mips).rgb;
	float2 env_brdf = GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(roughness, metallic, NV), 0);
	return ambient * diffuse_color + reflection * (f0 * env_brdf.x + env_brdf.y);
}
