#ifndef COMMON_HLSL
#define COMMON_HLSL

static const float PI = 3.14159265358979f;

static const float MIN_ROUGHNESS = 0.03f;
/*
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

*/
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


float2 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return res.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
}


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


float D_GGX(float roughness, float NoH)
{
	float a = NoH * roughness;
	float k = roughness / (1.0 - NoH * NoH + a * a);
	return k * k;
}

float D_GGX_Divide_Pi(float roughness, float NdotH)
{
	return D_GGX(roughness, NdotH) / PI;
}

float3x3 CalculateTangent(float3 normal)
{
	float3 UpVector = abs(normal.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, normal));
	float3 TangentY = cross(normal, TangentX);
	return float3x3(TangentX, TangentY, normal);
}

float3 TangentToWorld(float3 v, float3 normal)
{
	return mul(v, CalculateTangent(normal));
}


float GGX_Distribution(float cosThetaNH, float alpha) {
	float alpha2 = alpha * alpha;
	float NH_sqr = saturate(cosThetaNH * cosThetaNH);
	float den = NH_sqr * alpha2 + (1.0 - NH_sqr);
	return alpha2 / (PI * den * den);
}

float4 ImportanceSampleGGX(float2 E, float a) {
	float m2 = a * a;

	float Phi = 2 * PI * E.x;
	float CosTheta = sqrt((1 - E.y) / (1 + (m2 - 1) * E.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float d = (CosTheta * m2 - CosTheta) * CosTheta + 1;
	float D = m2 / (PI * d * d);

	float PDF = D * CosTheta;

	return float4(H, PDF);
}


float4 ImportanceSampleGGX(float2 Xi, float a, float3x3 TangentMatrix)
{
	float4 s = ImportanceSampleGGX(Xi, a);
	return float4(mul(s.xyz, TangentMatrix), s.w);
}
 
float4 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
	float3x3 space = CalculateTangent(N);
	return ImportanceSampleGGX(Xi, Roughness, space);
}

float GGX_PartialGeometry(float cosThetaN, float alpha) {
	float cosTheta_sqr = saturate(cosThetaN * cosThetaN);
	float tan2 = (1 - cosTheta_sqr) / cosTheta_sqr;
	float GP = 2 / (1 + sqrt(1 + alpha * alpha * tan2));
	return GP;
}

float4 CookTorrance_GGX_sample(float roug_sqr, float metalness, float3 n, float3 l, float3 v) {

	n = normalize(n);
	v = normalize(v);
	l = normalize(l);
	float3 h = normalize(v + l);
	//precompute dots
	float NL = dot(n, l);
	if (NL <= 0.0) return 0.0;
	float NV = dot(n, v);
	if (NV <= 0.0) return 0.0;
	float NH = dot(n, h);
	float HV = dot(h, v);


	//calc coefficients
	float G = GGX_PartialGeometry(NV, roug_sqr) * GGX_PartialGeometry(NL, roug_sqr);
	float3 F = FresnelSchlick(metalness, HV);

	float D = GGX_Distribution(NH, roug_sqr); //вот тут собственно мы добавили вычисление D
	float pdf = D * NH / (4.0 * HV); //и вычисление самой pdf

	float3 specK = max(0.0, G * F * HV / (NV * NH));
	return float4(specK, pdf);
}


float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
	float a2 = a * a;
	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
	float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
	return 0.5 / (GGXV + GGXL);
}


float GetReflCoeff(float Roughness, float Metallic, float3 l, float3 v, float3 n)
{
	float3 h = normalize(v + l);

	float NoV = abs(dot(n, v)) + 1e-5;
	float NoL = saturate(dot(n, l));
	float NoH = saturate(dot(n, h));
	float LoH = saturate(dot(l, h));


	float D = D_GGX_Divide_Pi(Roughness, NoH);
	float3  F = FresnelSchlick(Metallic, LoH);
	float V = V_SmithGGXCorrelated(NoV, NoL, Roughness);


	float3 Fr = max(0, D * V * F * 0.25 / NoV);

	return Fr;
}

float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

float GetRandom(float2 tc, float globaltime)
{
	float time = frac(globaltime) * 5;// +i;
	float sini = sin(time * 220 + float(tc.x));
	float cosi = cos(time * 220 + float(tc.y));
	float rand = rnd(float2(sini, cosi));

	return rand;
}

float2 GetRandom2(float2 tc, float globaltime)
{
	float rand = GetRandom(tc, globaltime);


	float rcos = cos(16.14 * rand);
	float rsin = sin(16.14 * rand);
	float rand2 = rnd(float2(rcos, rsin));

	return float2(rand, rand2);
}


float3 GetRandomDir(float2 tc, float3 normal, float roughness, float globaltime)
{
	float2 Xi = GetRandom2(tc, globaltime);
	float3 H = ImportanceSampleGGX(Xi, roughness, normal);
	return H;
}


float G1V_Epic(float a, float NoV)
{
	return NoV / (NoV * (1.0f - a) + a);
}

float G_Smith(float a, float NoV, float NoL)
{
	return G1V_Epic(a, NoV) * G1V_Epic(a, NoL);
}

//-------------------------------------------------------------------------------------------------
// Helper for computing the GGX visibility term
//-------------------------------------------------------------------------------------------------
float GGX_V1(in float m2, in float nDotX)
{
	return 1.0f / (nDotX + sqrt(m2 + (1 - m2) * nDotX * nDotX));
}

//-------------------------------------------------------------------------------------------------
// Computes the GGX visibility term
//-------------------------------------------------------------------------------------------------
float GGXVisibility(in float m2, in float nDotL, in float nDotV)
{
	return GGX_V1(m2, nDotL) * GGX_V1(m2, nDotV);
}

//-------------------------------------------------------------------------------------------------
// Computes the specular term using a GGX microfacet distribution, with a matching
// geometry factor and visibility term. Based on "Microfacet Models for Refraction Through
// Rough Surfaces" [Walter 07]. m is roughness, n is the surface normal, h is the half vector,
// l is the direction to the light source, and specAlbedo is the RGB specular albedo
//-------------------------------------------------------------------------------------------------
float GGX_Specular(in float m, in float3 n, in float3 h, in float3 v, in float3 l)
{
	float nDotH = saturate(dot(n, h));
	float nDotL = saturate(dot(n, l));
	float nDotV = saturate(dot(n, v));

	float nDotH2 = nDotH * nDotH;
	float m2 = m;// *m;

	// Calculate the distribution term
	float d = m2 / (PI * pow(nDotH * nDotH * (m2 - 1) + 1, 2.0f));

	// Calculate the matching visibility term
	float v1i = GGX_V1(m2, nDotL);
	float v1o = GGX_V1(m2, nDotV);
	float vis = v1i * v1o;

	return d * GGXVisibility(m2, nDotL, nDotV);
}




// http://jcgt.org/published/0007/04/01/paper.pdf by Eric Heitz
// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
float3 SampleGGXVNDF(float3 Ve, float alpha_x, float alpha_y, float U1, float U2) {
    // Section 3.2: transforming the view direction to the hemisphere configuration
    float3 Vh = normalize(float3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    float3 T1 = lensq > 0 ? float3(-Vh.y, Vh.x, 0) * rsqrt(lensq) : float3(1, 0, 0);
    float3 T2 = cross(Vh, T1);
    // Section 4.2: parameterization of the projected area
    float r = sqrt(U1);
    float phi = 2.0 * PI * U2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;
    // Section 4.3: reprojection onto hemisphere
    float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;
    // Section 3.4: transforming the normal back to the ellipsoid configuration
    float3 Ne = normalize(float3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
    return Ne;
}

float3 Sample_GGX_VNDF_Ellipsoid(float3 Ve, float alpha_x, float alpha_y, float U1, float U2) {
    return SampleGGXVNDF(Ve, alpha_x, alpha_y, U1, U2);
}

float3 Sample_GGX_VNDF_Hemisphere(float3 Ve, float alpha, float U1, float U2) {
    return Sample_GGX_VNDF_Ellipsoid(Ve, alpha, alpha, U1, U2);
}

float3x3 CreateTBN(float3 N) {
    float3 U;
    if (abs(N.z) > 0.0) {
        float k = sqrt(N.y * N.y + N.z * N.z);
        U.x = 0.0; U.y = -N.z / k; U.z = N.y / k;
    }
    else {
        float k = sqrt(N.x * N.x + N.y * N.y);
        U.x = N.y / k; U.y = -N.x / k; U.z = 0.0;
    }

    float3x3 TBN;
    TBN[0] = U;
    TBN[1] = cross(N, U);
    TBN[2] = N;
    return transpose(TBN);
}

float3 SampleReflectionVector(float3 view_direction, float3 normal, float roughness, float2 random) {
    float3x3 tbn_transform = CreateTBN(normal);
    float3 view_direction_tbn = mul(-view_direction, tbn_transform);

    float2 u =random;// SampleRandomVector2D(dispatch_thread_id);
    
    float3 sampled_normal_tbn = Sample_GGX_VNDF_Hemisphere(view_direction_tbn, roughness, u.x, u.y);
    #ifdef PERFECT_REFLECTIONS
        sampled_normal_tbn = float3(0, 0, 1); // Overwrite normal sample to produce perfect reflection.
    #endif
    
    float3 reflected_direction_tbn = reflect(-view_direction_tbn, sampled_normal_tbn);

    // Transform reflected_direction back to the initial space.
    float3x3 inv_tbn_transform = transpose(tbn_transform);
    return mul(reflected_direction_tbn, inv_tbn_transform);
}



#endif