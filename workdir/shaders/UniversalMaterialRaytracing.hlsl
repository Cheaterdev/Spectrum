#include "autogen/FrameInfo.h"
#include "autogen/MaterialInfo.h"
#include "autogen/SceneData.h"
#include "autogen/Raytracing.h"

#include "autogen/tables/RayPayload.h"
#include "autogen/tables/ShadowPayload.h"

#include "autogen/tables/Triangle.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelInfo.h"

#include "autogen/rtx/ShadowPass.h"
#include "autogen/rtx/ColorPass.h"
#include "autogen/tables/ColorShadowPayload.h"
#include "autogen/rtx/ColorShadowPass.h"

#include "Common.hlsl"
//#define REFRACTION
#define Sampler linearSampler
#define GetMaterialInfo CreateMaterialInfo

typedef BuiltInTriangleIntersectionAttributes MyAttributes;



float4 get_voxel(float3 pos, float level)
{
	float4 color = CreateVoxelScreen().GetVoxels().SampleLevel(linearClampSampler, pos, level);
	return color;
}




Texture2D get_texture(uint i)
{
	return GetMaterialInfo().GetTextures(i);

	//	return textures[i /*+ texture_offset*/];
}

#ifdef BUILD_FUNC_PS
#define sample(tex, s,  tc, lod) get_texture(tex).Sample(s, tc);
#else
float4 sample(uint itex, SamplerState s, float2 tc, float lod)
{
    Texture2D tex = get_texture(itex);
	
	uint2 size;
	tex.GetDimensions(size.x, size.y);
	lod += 0.5 * log2(size.x * size.y);

	return tex.SampleLevel(s, tc, lod);
}
#endif 

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f, out float4 g, out float h, out float ior, float lod);


// Evaluate the material at the current hit and return just what a shadow ray
// needs: surface colour (tint) and opacity. Mirrors the setup in the color hit.
void ShadowSurface(in MyAttributes attr, out float4 color, out float opacity)
{
	SceneData sceneData = CreateSceneData();
	RaytraceInstanceInfo instance = sceneData.GetRaytraceInstanceInfo()[InstanceID()];

	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

	uint id0 = instance.GetIndices()[PrimitiveIndex() * 3];
	uint id1 = instance.GetIndices()[PrimitiveIndex() * 3 + 1];
	uint id2 = instance.GetIndices()[PrimitiveIndex() * 3 + 2];

	mesh_vertex_input v0 = instance.GetVertexes()[id0];
	mesh_vertex_input v1 = instance.GetVertexes()[id1];
	mesh_vertex_input v2 = instance.GetVertexes()[id2];

	Triangle t;
	t.init(v0, v1, v2, barycentrics);
	t.v.pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

	color = 1; opacity = 1;
	float  metallic = 1, roughness = 1, refraction = 1;
	float4 normal = 0, glow = 0;
	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, opacity, refraction, t.lod);
}


// Transparent-aware shadow closest-hit (per material). Opaque materials block the
// light; transparent ones tint the transmittance by their colour/opacity and
// continue the ray toward the light through further layers.
[shader("closesthit")]
void ColorShadowClosestHitShader([raypayload] inout ColorShadowPayload payload, in MyAttributes attr)
{
	payload.dist = RayTCurrent(); // report the occluder distance to the caller.

#ifdef TRANSPARENT
	float4 color; float opacity;
	ShadowSurface(attr, color, opacity);

	// Light through the glass is tinted by its colour; opacity darkens it (full
	// opacity == opaque == blocks). The caller advances past and keeps tracing.
	payload.transmittance *= color.rgb * (1.0 - saturate(opacity));
#else
	payload.transmittance = 0.0; // opaque occluder -> full shadow
#endif
}



[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{


	FrameInfo frame = CreateFrameInfo();
	SceneData sceneData = CreateSceneData();
	Raytracing raytracing = CreateRaytracing();

	RaytraceInstanceInfo instance = sceneData.GetRaytraceInstanceInfo()[InstanceID()];

	
	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);


	uint id0 = instance.GetIndices()[PrimitiveIndex() * 3]; 
	uint id1 = instance.GetIndices()[PrimitiveIndex() * 3 + 1];
	uint id2 = instance.GetIndices()[PrimitiveIndex() * 3 + 2];

	mesh_vertex_input vertex0 = instance.GetVertexes()[id0];
	mesh_vertex_input vertex1 = instance.GetVertexes()[id1];
	mesh_vertex_input vertex2 = instance.GetVertexes()[id2];


	Triangle t;
	t.init(vertex0, vertex1, vertex2, barycentrics);

	//if (HitKind() != HIT_KIND_TRIANGLE_FRONT_FACE)
	//	t.v.normal = -t.v.normal;

	t.v.pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

	float kR = 0.9;
	float3 refl = reflect(WorldRayDirection(), t.v.normal);
	float3 refr = refract(WorldRayDirection(), t.v.normal, HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? kR : (1.0 / kR));
	float4 color = 1;
	float metallic = 1;
	float roughness = 1;
	float4 normal = 0;
	float4 glow = 0;
	float opacity = 1;
	float refraction = 1;


	t.lod += log2(abs(payload.cone.width + payload.cone.angle * RayTCurrent()));
	t.lod -= log2(abs(dot(normalize(WorldRayDirection()), t.v.normal)));


	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, opacity, refraction, t.lod);

	// Transparent-aware sun visibility (float3 transmittance instead of a bool).
	// Iterate the shadow ray through transparent occluders (advancing past each
	// hit) until it hits something opaque (fully black) or misses (reaches the
	// light). The loop keeps every trace at the same recursion depth, so however
	// many glass layers there are it costs only one level. Gated at shallow color
	// recursion so deep refraction bounces don't spawn shadow rays.
	float3 sun_vis = 1.0;
	if (payload.recursion < 2)
	{
		float3 s_origin = t.v.pos;
		float3 s_dir    = normalize(frame.GetSunDir().xyz);

		[loop]
		for (int si = 0; si < 16; si++)
		{
			[raypayload] ColorShadowPayload sp;
			sp.transmittance = sun_vis;
			sp.dist          = -1.0;

			RayDesc shadow_ray;
			shadow_ray.Origin    = s_origin;
			shadow_ray.Direction = s_dir;
			shadow_ray.TMin      = 0.001;
			shadow_ray.TMax      = 10000.0;
			ColorShadowPass(raytracing.GetScene(), shadow_ray, RAY_FLAG_NONE, sp);

			sun_vis = sp.transmittance;

			if (sp.dist < 0.0)         break; // missed -> reached the light
			if (all(sun_vis < 0.01))   break; // fully occluded -> black shadow

			// Advance just past the transparent hit and keep tracing.
			s_origin += s_dir * (sp.dist + 0.01);
		}
	}

	float NdotL = saturate(dot(t.v.normal, normalize(frame.GetSunDir().xyz)));
	payload.color = float4(color.rgb * NdotL * sun_vis, 1.0);
	payload.dist  = RayTCurrent();

#ifdef TRANSPARENT
	// Transparent surface: continue a refracted color ray through the surface
	// (Snell's law with the material's refraction index) and blend it behind the
	// surface shading according to opacity. Guarded by recursion depth.
	if (payload.recursion < 6)
	{
		float3 wi = normalize(WorldRayDirection());
		float3 N  = normalize(t.v.normal);

		// Orient the normal against the incoming ray. entering = hitting the outer
		// (air-side) face; !entering = the ray is leaving the medium (back face).
		bool entering = dot(wi, N) < 0.0;
		if (!entering) N = -N;

		// eta = n_from / n_to.
		float eta = entering ? (1.0 / refraction) : refraction;

		// Fresnel (Schlick) reflectance for the air/medium interface: grazing
		// angles reflect more, near-normal angles transmit more.
		float R0   = (refraction - 1.0) / (refraction + 1.0);
		R0        *= R0;
		float cosI = saturate(dot(-wi, N));
		float F    = R0 + (1.0 - R0) * pow(1.0 - cosI, 5.0);

		float3 refr_dir = refract(wi, N, eta);
		bool   tir      = dot(refr_dir, refr_dir) < 1e-4;
		if (tir) F = 1.0; // total internal reflection: everything reflects

		// Beer-Lambert absorption coefficient from the glass tint.
		const float absorption = 0.35; // tune: higher = tints faster with distance
		float3 sigma = (1.0 - saturate(color.rgb)) * absorption;

		float3 reflected = 0;
		float3 refracted = 0;

		// Reflection ray. Only branch at shallow recursion to keep the ray tree
		// from exploding; deeper bounces transmit only.
		if (F > 0.01 && payload.recursion < 3)
		{
			[raypayload] RayPayload rp;
			rp.init();
			rp.recursion = payload.recursion + 1;
			rp.cone      = payload.cone;

			RayDesc rr;
			rr.Origin    = t.v.pos;
			rr.Direction = reflect(wi, N);
			rr.TMin      = 0.001;
			rr.TMax      = 10000.0;
			ColorPass(raytracing.GetScene(), rr, RAY_FLAG_NONE, rp);

			reflected = rp.color.rgb;
			// A reflection off the inner boundary stays inside the medium.
			if (!entering) reflected *= exp(-sigma * rp.dist);
		}

		// Refraction ray (skipped on TIR or when its Fresnel weight is negligible).
		if (!tir && F < 0.99)
		{
			[raypayload] RayPayload rp;
			rp.init();
			rp.recursion = payload.recursion + 1;
			rp.cone      = payload.cone;

			RayDesc rr;
			rr.Origin    = t.v.pos;
			rr.Direction = refr_dir;
			rr.TMin      = 0.001;
			rr.TMax      = 10000.0;
			ColorPass(raytracing.GetScene(), rr, RAY_FLAG_NONE, rp);

			refracted = rp.color.rgb;
			// Refracting inward travels through the medium -> Beer-Lambert absorb.
			if (entering) refracted *= exp(-sigma * rp.dist);
		}

		// Fresnel blend of reflection and transmission, then surface coverage.
		float3 combined = lerp(refracted, reflected, F);
		payload.color = lerp(float4(combined, 1.0), payload.color, saturate(opacity));
	}
#endif
}