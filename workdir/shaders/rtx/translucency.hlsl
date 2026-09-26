#ifndef TRANSLUCENCY_HLSL
#define TRANSLUCENCY_HLSL

// TranslucentRTX: full-screen raygen compositing TransparencyMode::Translucent
// geometry (glass, water) over the lit scene. Translucent instances live under
// RTInstanceMask::Translucent only, so the primary query is bounded by opaque
// depth and misses cheaply wherever there is no glass.
//
// Thin-walled (thickness == 0): the ray continues straight, so what lies behind
// is exactly this pixel's lit scene colour -- no ray needed for it.
// Solid (thickness > 0): refracted in and out; the traced in-medium distance
// drives Beer-Lambert absorption. Once bent, the ray needs a real scene query.
#include "../autogen/FrameInfo.h"
#include "../autogen/Raytracing.h"
#include "../autogen/TranslucentRTXData.h"
#include "../autogen/tables/RayPayload.h"
#include "../autogen/tables/TranslucentPayload.h"
#include "../autogen/rtx/ColorPass.h"
#include "../autogen/rtx/TranslucentPass.h"

static const uint  TRANSLUCENT_MAX_SEGMENTS = 8;
static const float TRANSLUCENT_RAY_EPS = 0.002;
static const float TRANSLUCENT_FAR = 100000.0;

float3 translucent_unproject(float d, float2 uv, float4x4 inv_view_proj)
{
	float4 p = mul(inv_view_proj, float4(uv * float2(2, -2) + float2(-1, 1), d, 1));
	return p.xyz / p.w;
}

// Unpolarized Fresnel reflectance for a dielectric interface, n1 -> n2.
// Returns 1 on total internal reflection.
float translucent_fresnel(float cos_i, float n1, float n2)
{
	float eta = n1 / n2;
	float sin2_t = eta * eta * saturate(1 - cos_i * cos_i);
	if (sin2_t >= 1)
		return 1;
	float cos_t = sqrt(1 - sin2_t);
	float rs = (n1 * cos_i - n2 * cos_t) / (n1 * cos_i + n2 * cos_t);
	float rp = (n1 * cos_t - n2 * cos_i) / (n1 * cos_t + n2 * cos_i);
	return 0.5 * (rs * rs + rp * rp);
}

// Below this GGX alpha the surface is treated as perfectly smooth: no random
// perturbation, so smooth glass stays noise-free.
static const float TRANSLUCENT_MIN_ALPHA = 0.001;

uint translucent_pcg(uint v)
{
	uint state = v * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float2 translucent_rand2(inout uint rng)
{
	rng = translucent_pcg(rng);
	float a = rng * (1.0 / 4294967296.0);
	rng = translucent_pcg(rng);
	float b = rng * (1.0 / 4294967296.0);
	return float2(a, b);
}

// GGX visible-normal sampling (Heitz 2018, "Sampling the GGX Distribution of
// Visible Normals"). N and V on the same side; returns a world-space
// microfacet normal. One random normal per pixel per frame: TAA/the upscaler
// integrates it into the rough (frosted) look.
float3 translucent_sample_microfacet(float3 N, float3 V, float alpha, inout uint rng)
{
	if (alpha < TRANSLUCENT_MIN_ALPHA)
		return N;

	// Orthonormal basis around N (Duff et al. 2017).
	float s = N.z >= 0 ? 1.0 : -1.0;
	float a = -1.0 / (s + N.z);
	float b = N.x * N.y * a;
	float3 T = float3(1 + s * N.x * N.x * a, s * b, -s * N.x);
	float3 B = float3(b, s + N.y * N.y * a, -N.y);

	float3 Ve = normalize(float3(dot(V, T), dot(V, B), max(dot(V, N), 1e-4)));
	float2 u = translucent_rand2(rng);

	float3 Vh = normalize(float3(alpha * Ve.x, alpha * Ve.y, Ve.z));
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	float3 T1 = lensq > 0 ? float3(-Vh.y, Vh.x, 0) * rsqrt(lensq) : float3(1, 0, 0);
	float3 T2 = cross(Vh, T1);
	float r = sqrt(u.x);
	float phi = 6.28318530718 * u.y;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float h = 0.5 * (1 + Vh.z);
	t2 = (1 - h) * sqrt(1 - t1 * t1) + h * t2;
	float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0, 1 - t1 * t1 - t2 * t2)) * Vh;
	float3 m = normalize(float3(alpha * Nh.x, alpha * Nh.y, max(Nh.z, 0)));

	return normalize(m.x * T + m.y * B + m.z * N);
}

TranslucentPayload trace_translucent(RaytracingAccelerationStructure scene, float3 origin, float3 dir, float tmax)
{
	TranslucentPayload hit;
	hit.dist = -1;

	RayDesc ray;
	ray.Origin    = origin;
	ray.Direction = dir;
	ray.TMin      = 0;
	ray.TMax      = tmax;
	// No culling: solid glass is hit from inside on the way out.
	TranslucentPass(scene, ray, RAY_FLAG_NONE, (uint)RTInstanceMask::Translucent, hit);
	return hit;
}

// Radiance arriving along a ray that is no longer the primary pixel ray.
// Prefers the already-lit scene colour when the hit point is visible on screen
// -- ColorPass shading is direct light only (no GI), so using it for things
// the camera also sees directly would make them look darker through glass
// than next to it.
float3 shade_opaque(RaytracingAccelerationStructure scene, float3 origin, float3 dir,
	Camera camera, TranslucentRTXData data, uint2 dims, out float hit_t)
{
	RayPayload p = CreateRayPayload();

	RayDesc ray;
	ray.Origin    = origin;
	ray.Direction = dir;
	ray.TMin      = 0;
	ray.TMax      = TRANSLUCENT_FAR;
	ColorPass(scene, ray, RAY_FLAG_NONE, (uint)RTInstanceMask::Opaque, p);

	hit_t = p.dist;
	if (p.dist >= TRANSLUCENT_FAR * 0.5)
		return p.color.rgb;

	float3 hit_pos = origin + dir * p.dist;
	float4 clip = mul(camera.GetViewProj(), float4(hit_pos, 1));
	if (clip.w > 0)
	{
		float2 uv = clip.xy / clip.w * float2(0.5, -0.5) + 0.5;
		if (all(uv > 0) && all(uv < 1))
		{
			uint2 px = min(uint2(uv * dims), dims - 1);
			float d = data.GetDepth()[px];
			if (d > 0)
			{
				float3 screen_pos = translucent_unproject(d, uv, camera.GetInvViewProj());
				float view_dist = distance(camera.GetPosition().xyz, hit_pos);
				if (distance(screen_pos, hit_pos) < 0.01 * view_dist + 0.02)
					return data.GetScene_color()[px].rgb;
			}
		}
	}

	return p.color.rgb;
}

[shader("raygeneration")]
void TranslucentRaygenShader()
{
	uint2 px   = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;

	const FrameInfo frame = CreateFrameInfo();
	const Camera camera = frame.GetCamera();
	const TranslucentRTXData data = CreateTranslucentRTXData();
	RaytracingAccelerationStructure scene = CreateRaytracing().GetScene();
	RWTexture2D<float4> output = data.GetOutput();

	float4 background = data.GetScene_color()[px];

	float2 uv = (px + 0.5) / float2(dims);
	float3 origin = camera.GetPosition().xyz;
	// Mid-range depth rather than near/far: with reverse-Z and an infinite far
	// plane, unprojecting d == 0 gives w == 0.
	float3 dir = normalize(translucent_unproject(0.5, uv, camera.GetInvViewProj()) - origin);

	// Reverse-Z: 0 is sky (same test Sky's own sky_result uses).
	float d = data.GetDepth()[px];
	float t_opaque = d > 0 ? length(translucent_unproject(d, uv, camera.GetInvViewProj()) - origin) : TRANSLUCENT_FAR;

	TranslucentPayload hit = trace_translucent(scene, origin, dir, t_opaque);
	if (hit.dist < 0)
	{
		output[px] = background;
		return;
	}

	float3 sun_dir = normalize(frame.GetSunDir().xyz);

	float3 radiance   = 0;
	float3 throughput = 1;
	float3 pos        = origin;
	// Still on the primary pixel ray: whatever lies behind is background.
	bool   straight   = true;
	float  traveled   = 0;
	bool   inside     = false;
	float3 sigma      = 0;
	bool   resolved   = false;
	uint   rng        = translucent_pcg(px.x + px.y * 65536u) ^ translucent_pcg(asuint(frame.GetTime().y));

	[loop]
	for (uint seg = 0; seg < TRANSLUCENT_MAX_SEGMENTS; seg++)
	{

		float3 hit_pos = pos + dir * hit.dist;
		if (straight)
			traveled += hit.dist;
		if (inside)
			throughput *= exp(-sigma * hit.dist);

		float3 N = hit.normal;
		bool entering = hit.front_face != 0;
		if (!entering)
			N = -N;

		// Graph refraction output; its unwired default of 1.0 means no interface.
		float  ior          = max(hit.ior, 1.0001);
		float  transmission = saturate(hit.transmission);
		float3 albedo       = saturate(hit.albedo);
		bool   thin         = hit.thickness <= 0;
		float  roughness    = saturate(hit.roughness);
		float  alpha        = roughness * roughness;

		// Everything at this interface (Fresnel, reflection, refraction) uses
		// the sampled microfacet normal; N stays the macro surface for "which
		// side is which" checks.
		float3 m     = translucent_sample_microfacet(N, -dir, alpha, rng);
		float  cos_i = saturate(dot(-dir, m));

		// Two interfaces for thin glass, one for a volume boundary.
		float F = thin ? translucent_fresnel(cos_i, 1, ior) : translucent_fresnel(cos_i, entering ? 1 : ior, entering ? ior : 1);
		float R = thin ? 2 * F / (1 + F) : F;

		// Reflection and the non-transmitted diffuse part only at the first
		// surface: deeper ones would each cost another scene ray per pixel.
		if (seg == 0)
		{
			float3 refl_dir = reflect(dir, m);
			if (dot(refl_dir, N) <= 0)
				refl_dir = reflect(dir, N);

			float refl_t;
			radiance += throughput * R * shade_opaque(scene, hit_pos + N * TRANSLUCENT_RAY_EPS, refl_dir, camera, data, dims, refl_t);
			radiance += throughput * (1 - R) * (1 - transmission) * albedo * saturate(dot(N, sun_dir));
		}

		if (thin)
		{
			throughput *= (1 - R) * transmission * albedo;

			if (alpha >= TRANSLUCENT_MIN_ALPHA)
			{
				// Frosted: the slab's two faces get independent microfacets, so
				// the ray leaves in a different direction (with both equal to N
				// this reduces to the smooth case: straight through).
				float3 d1 = refract(dir, m, 1 / ior);
				if (dot(d1, d1) > 1e-6)
				{
					float3 m2 = translucent_sample_microfacet(N, -d1, alpha, rng);
					float3 d2 = refract(d1, m2, ior);
					if (dot(d2, d2) > 1e-6 && dot(d2, N) < 0)
					{
						dir = normalize(d2);
						straight = false;
					}
				}
			}

			pos = hit_pos - N * TRANSLUCENT_RAY_EPS;
			if (straight)
				traveled += TRANSLUCENT_RAY_EPS;
		}
		else
		{
			float3 refr = refract(dir, m, entering ? 1 / ior : ior);
			if (F >= 1 || dot(refr, refr) < 1e-6 || dot(refr, N) >= 0)
			{
				// Total internal reflection (or a microfacet that can't
				// transmit toward the far side): stays on this side.
				float3 bounced = reflect(dir, m);
				dir = dot(bounced, N) > 0 ? bounced : reflect(dir, N);
				pos = hit_pos + N * TRANSLUCENT_RAY_EPS;
			}
			else
			{
				throughput *= 1 - F;
				if (entering)
				{
					throughput *= transmission;
					sigma = -log(max(albedo, 0.001)) / max(hit.absorption_distance, 0.0001);
				}
				dir = normalize(refr);
				pos = hit_pos - N * TRANSLUCENT_RAY_EPS;
				inside = entering;
			}
			straight = false;
		}

		if (max(throughput.x, max(throughput.y, throughput.z)) < 0.001)
		{
			resolved = true;
			break;
		}

		if (straight)
		{
			hit = trace_translucent(scene, pos, dir, max(t_opaque - traveled, 0));
			if (hit.dist < 0)
			{
				radiance += throughput * background.rgb;
				resolved = true;
				break;
			}
			continue;
		}

		if (inside)
		{
			hit = trace_translucent(scene, pos, dir, TRANSLUCENT_FAR);
			if (hit.dist >= 0)
				continue;
			// Open mesh (no back face to exit through): treat it as exited here.
			inside = false;
		}

		float opaque_t;
		float3 opaque = shade_opaque(scene, pos, dir, camera, data, dims, opaque_t);
		hit = trace_translucent(scene, pos, dir, opaque_t);
		if (hit.dist >= 0)
			continue;

		radiance += throughput * opaque;
		resolved = true;
		break;
	}

	// Out of segments: whatever is left is closest to the unrefracted scene.
	if (!resolved)
		radiance += throughput * background.rgb;

	output[px] = float4(radiance, background.a);
}

[shader("miss")]
void TranslucentMissShader([raypayload] inout TranslucentPayload payload)
{
	payload.dist = -1;
}

#endif // TRANSLUCENCY_HLSL
