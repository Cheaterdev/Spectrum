// Needs FrameInfo.h (the BRDF lookup) and the common samplers included first.
// The baked sky as the indirect light of a pipeline with no GI (ray tracing, voxel GI): diffuse ambient from the
// cosine-convolved cube (sky_cubemap_filtered_diffuse), and the reflection a metal shows from the
// specular-filtered one (sky_cubemap_filtered). Without it a metallic material (spec_to_metallic) has no
// diffuse term and renders black.
//
// The material (common/shading_model.hlsl) decides what else the sky does for a pixel: the coat takes its
// Fresnel share and reflects the sky itself, sheen scatters ambient light, a thin film recolours the
// reflection. The diffuse and specular halves are separate so a pass can run the subsurface blur between
// them (it must blur only the diffuse light).

#include "lobes.hlsl"

// The mip of the specular-filtered cube for a roughness (alpha, the perceptual roughness squared). Mip m
// was filtered at perceptual roughness (m + 0.5) / mip_count (sky/cubemap_down.hlsl, CS), so invert that.
float sky_mip(TextureCube<float4> sky, float alpha)
{
	uint width, height, mips;
	sky.GetDimensions(0, width, height, mips);

	return clamp(sqrt(alpha) * mips - 0.5, 0, mips - 1);
}

float3 sky_diffuse_light(TextureCube<float4> sky_diffuse, float4 material, float3 albedo, float metallic, float3 normal, float3 view)
{
	float3 ambient = sky_diffuse.SampleLevel(linearClampSampler, normal, 0).rgb;
	float3 diffuse = ambient * albedo * (1 - metallic) * coat_base_attenuation(material, normal, view);

	return diffuse + sheen_indirect(material, normal, view, albedo, ambient);
}

// `roughness` is the perceptual roughness squared, as the other lighting passes pass it.
float3 sky_specular_light(TextureCube<float4> sky, float4 material, float3 albedo, float metallic, float3 normal, float3 view, float roughness)
{
	float NV = saturate(dot(normal, view));
	float3 f0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

	if (material_model(material) == SHADING_MODEL_THINFILM)
		f0 = iridescent_f0(NV, thinfilm_thickness(material), thinfilm_ior(material), f0);

	float3 mirror = reflect(-view, normal);

	// The base reflection of an anisotropic surface is stretched; the sun term has its own lobe.
	if (material_model(material) == SHADING_MODEL_ANISOTROPIC)
	{
		float3 tangent = aniso_decode_tangent(normal, material_params(material).y);
		mirror = reflect(-view, aniso_bent_normal(normal, view, tangent, material_params(material).x));
	}

	float3 reflection = sky.SampleLevel(linearClampSampler, mirror, sky_mip(sky, roughness)).rgb;
	float2 env_brdf = GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(roughness, metallic, 0.5 + 0.5 * NV), 0);

	float3 result = reflection * (f0 * env_brdf.x + env_brdf.y) * coat_base_attenuation(material, normal, view);

	if (material_model(material) == SHADING_MODEL_CLEARCOAT)
	{
		float coat_roughness = material_params(material).y;
		float3 coat_radiance = sky.SampleLevel(linearClampSampler, mirror, sky_mip(sky, coat_roughness * coat_roughness)).rgb;
		result += coat_reflection(material, normal, view, coat_radiance);
	}

	return result;
}

float3 sky_indirect_light(TextureCube<float4> sky, TextureCube<float4> sky_diffuse, float4 material, float3 albedo, float metallic, float3 normal, float3 view, float roughness)
{
	return sky_diffuse_light(sky_diffuse, material, albedo, metallic, normal, view)
	     + sky_specular_light(sky, material, albedo, metallic, normal, view, roughness);
}

// A plain metallic-roughness surface.
float3 sky_indirect_light(TextureCube<float4> sky, TextureCube<float4> sky_diffuse, float3 albedo, float metallic, float3 normal, float3 view, float roughness)
{
	return sky_indirect_light(sky, sky_diffuse, pack_material(SHADING_MODEL_STANDARD, 0.0), albedo, metallic, normal, view, roughness);
}
