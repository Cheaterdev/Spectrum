#pragma once

float chiGGX(float v)
{
	return v > 0 ? 1 : 0;
}

float GGX_PartialGeometryTerm(float3 v, float3 n, float3 h, float alpha)
{
	float VoH2 = saturate(dot(v, h));
	float chi = chiGGX(VoH2 / saturate(dot(v, n)));
	VoH2 = VoH2 * VoH2;
	float tan2 = (1 - VoH2) / VoH2;
	return (chi * 2) / (1 + sqrt(1 + alpha * alpha * tan2));
}

float3 Fresnel_Schlick(float cosT, float3 F0)
{
	return F0 + (1 - F0) * pow(1 - cosT, 5);
}

float GGX_Specular(float3 normal, float3 viewVector, float roughness, float3 F0, out float3 kS)
{
	float radiance = 0;
	float  NoV = saturate(dot(normal, viewVector));
	kS = 0;

	float3 sampleVector = normal;
	// Calculate the half vector
	float3 halfVector = normalize(normal + viewVector);
	//	float cosT = saturate(dot(sampleVector, normal));
	//	float sinT = sqrt(1 - cosT * cosT);

	// Calculate fresnel
	float3 fresnel = Fresnel_Schlick(clamp(dot(normal, viewVector),0.2,1), F0);


	// Geometry term
	float geometry = GGX_PartialGeometryTerm(viewVector, normal, halfVector, roughness);// *GGX_PartialGeometryTerm(sampleVector, normal, halfVector, roughness);
																						// Calculate the Cook-Torrance denominator
	float denominator = saturate(4 * (NoV * saturate(dot(halfVector, normal)) + 0.05));
	kS += fresnel;
	// Accumulate the radiance
	radiance = geometry * fresnel / denominator;


	return radiance;
}

float3 PBR( float3 gi, float3 reflection, float3 base_color, float3 normal, float3 v, float reflection_index, float roughness, float metallic)
{
//	return base_color*metallic;


	float ior = 1.4;// +surface.r;

					// Calculate colour at normal incidence
	float3 F0 = 0.04;// abs((1.0 - ior) / (1.0 + ior));
	F0 = F0 * F0;
	F0 = lerp(F0, base_color.rgb, metallic);

	//return float4(F0,1);
	float3 ks = 0;
	float coeff = GGX_Specular(normal, -v, roughness, F0, ks);

	float3 kd = (1 - ks) * (1 - metallic);
	// Calculate the diffuse contribution
	float3 specular = coeff*reflection;


	//return float4(coeff.xxx,1);
	return 1*kd * base_color*gi + ks*specular;


}