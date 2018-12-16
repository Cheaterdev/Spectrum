#ifndef COMMON_HLSL
#define COMMON_HLSL

static const float PI = 3.14159265358979f;

struct camera_info
{
	matrix view;
	matrix proj;
	matrix view_proj;
	matrix inv_view;
	matrix inv_proj;
	matrix inv_view_proj;
	float3 position;
	float unused;
	float3 direction;
	float unused2;
	float2 jitter; float2 unused3;
};


struct pixel_info
{
	float view_z;
	float3 pos;
	float3 normal;
	float3 albedo;
	float metallic;
	float roughness;

	float3 view;
	float3 reflection;
};

float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc, d, 1));
	return P.xyz / P.w;
}
float3 FresnelSchlick(float3 F0, float cosTheta) {
	return F0 + (1.0 - F0) * pow(1.0 - saturate(cosTheta), 5.0);
}

float calc_fresnel(float k0, float3 n, float3 v)
{
	float ndv = saturate(dot(n, -v));
	return k0 + (1 - k0) * pow(1 - ndv, 5);
	return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

float3 calc_color(pixel_info info, float3 light_dir, float3 light_color)
{
	float fresnel = calc_fresnel(info.roughness, info.normal, info.view);
	float s = pow(saturate(dot(info.reflection, light_dir)), 128 * fresnel);
	float3 diffuse = info.albedo * max(0, dot(light_dir, info.normal)) * (1 - fresnel);
	float3 reflection = 0;// info.specular * fresnel * 50 * s;
	return light_color * (1 * diffuse + reflection);
}



double radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return double(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 hammersley2d(uint i, uint N) {
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}

/*
float3 ImportanceSampleGGX(float2 vXi, float fRoughness, float3 vNormal)
{
	// Compute the local half vector
	float fA = fRoughness * fRoughness;
	float fPhi = 2.0f * PI * vXi.x;
	float fCosTheta = sqrt((1.0f - vXi.y) / (1.0f + (fA*fA - 1.0f) * vXi.y));
	float fSinTheta = sqrt(1.0f - fCosTheta * fCosTheta);
	float3 vHalf;
	vHalf.x = fSinTheta * cos(fPhi);
	vHalf.y = fSinTheta * sin(fPhi);
	vHalf.z = fCosTheta;

	// Compute a tangent frame and rotate the half vector to world space
	float3 vUp = abs(vNormal.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
	float3 vTangentX = normalize(cross(vUp, vNormal));
	float3 vTangentY = cross(vNormal, vTangentX);
	// Tangent to world space
	return vTangentX * vHalf.x + vTangentY * vHalf.y + vNormal * vHalf.z;
}

*/


float D_GGX(float roughness, float NdotH)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH2 = NdotH * NdotH;
	float f = 1.0f + (NdotH2 * (a2 - 1.0f));
	return a2 / (f * f);
}

float D_GGX_Divide_Pi(float roughness, float NdotH)
{
	return D_GGX(roughness, NdotH) / PI;
}



float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - min(1.0f, CosTheta * CosTheta));
	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;
	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}



float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N, float3 TangentX, float3 TangentY)
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - min(1.0f, CosTheta * CosTheta));
	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;


	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}
 
float G1V_Epic(float Roughness, float NoV)
{
	// no hotness remapping for env BRDF as suggested by Brian Karis
	float k = Roughness * Roughness;
	return NoV / (NoV * (1.0f - k) + k);
}

float G_Smith(float Roughness, float NoV, float NoL)
{
	return G1V_Epic(Roughness, NoV) * G1V_Epic(Roughness, NoL);
}



float GGX_PartialGeometry(float cosThetaN, float alpha) {
	float cosTheta_sqr = saturate(cosThetaN*cosThetaN);
	float tan2 = (1 - cosTheta_sqr) / cosTheta_sqr;
	float GP = 2 / (1 + sqrt(1 + alpha * alpha * tan2));
	return GP;
}

float GGX_Distribution(float cosThetaNH, float alpha) {
	float alpha2 = alpha * alpha;
	float NH_sqr = saturate(cosThetaNH * cosThetaNH);
	float den = NH_sqr * alpha2 + (1.0 - NH_sqr);
	return alpha2 / (PI * den * den);
}

float3 CookTorrance_GGX_sample( float3 l, pixel_info info, out float3 FK) {
	FK = 0.0;


	float3 n = normalize(info.normal);
	float3 v = normalize(info.view);
	 l = normalize(l);
	float3 h = normalize(v + l);
	//precompute dots
	float NL = dot(n, l);
	if (NL <= 0.0) return 0.0;
	float NV = dot(n, v);
	if (NV <= 0.0) return 0.0;
	float NH = dot(n, h);
	float HV = dot(h, v);

	//precompute roughness square
	float roug_sqr = info.roughness*info.roughness;

	//calc coefficients
	float G = GGX_PartialGeometry(NV, roug_sqr) * GGX_PartialGeometry(NL, roug_sqr);
	float D = GGX_Distribution(NH, roug_sqr);

	float3 F = FresnelSchlick(info.metallic, NV);
	FK = F;

	//mix
	float3 specK = G * D*F*0.25 / NV;
	return max(0.0, specK);
}


#endif