#include "autogen/FrameInfo.h"
#include "autogen/MaterialInfo.h"
#include "autogen/SceneData.h"
#include "autogen/Raytracing.h"

#include "autogen/tables/RayPayload.h"
#include "autogen/tables/ShadowPayload.h"

#include "autogen/tables/Triangle.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VSMShadowLookupData.h"

#include "autogen/rtx/ShadowPass.h"
#include "autogen/rtx/ColorPass.h"
#include "autogen/tables/ColorShadowPayload.h"
#include "autogen/rtx/ColorShadowPass.h"
#include "autogen/tables/TranslucentPayload.h"
// RayPayload::use_vsm_shadow's own opt-in cheap path (see its comment,
// raytracing.prism) -- get_shadow_vsm_simple, the same lean lookup VoxelGI's
// own Lighting pass uses (voxel_lighting.hlsl) for exactly the same reason
// (runs before VSM_BlockerClassify/VSM_ShadowResolve, so only the raw
// atlas+page-table lookup is available). No "../" here despite this file
// physically living in rtx/ -- every other include above is already
// relative to workdir/shaders/ directly, not to this file's own directory
// (material shaders preprocess this file as a virtual/synthetic buffer, not
// a real on-disk path, so directory-relative "../" resolution overshoots).
#include "shadows/vsm/vsm_impl_resolve.hlsl"

#include "common/common.hlsl"
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
#define sample(tex, s,  tc, lod) get_texture(tex).SampleBias(s, tc, GetFrameInfo().GetMipBias());
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

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f, out float4 g, out float h, out float ior, out float thickness, out float transmission, out float absorption_distance, float lod);


// Evaluate the material at the current hit and return just what a shadow ray
// needs: surface colour (tint) and opacity. Mirrors the setup in the color hit.
void ShadowSurface(in MyAttributes attr, out float4 color, out float opacity, out float surface_transmission)
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
	float  metallic = 1, roughness = 1, refraction = 1, thickness = 0, transmission = 1, absorption_distance = 1;
	float4 normal = 0, glow = 0;
	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, opacity, refraction, thickness, transmission, absorption_distance, t.lod);
	surface_transmission = transmission;
}


// RayPayload::use_vsm_shadow's own cheap path (see its comment,
// raytracing.prism): bridges VSMShadowLookupData (vsm.prism, bound by the pass
// that opted in) into the VSMConstants/VSMLighting shapes
// get_shadow_vsm_simple actually takes -- exactly the same field-by-field
// copy voxel_lighting.hlsl's own get_shadow() already does, for the same
// reason (a caller outside VSM's own passes only has the lean lookup
// available, not the full per-pass VSMConstants/VSMLighting tables).
float3 vsm_shadow_lookup(float3 wpos, float3 normal, float3 light_dir)
{
	VSMShadowLookup vsm_lookup = CreateVSMShadowLookupData().GetLookup();

	VSMConstants c = (VSMConstants)0;
	c.active_min      = vsm_lookup.active_min;
	c.active_max      = vsm_lookup.active_max;
	c.page_size       = vsm_lookup.page_size;
	c.pages_per_level = vsm_lookup.pages_per_level;
	c.light_view      = vsm_lookup.light_view;
	[unroll]
	for (int li = 0; li < 26; li++)
		c.level_info[li] = vsm_lookup.level_info[li];

	VSMLighting lighting = (VSMLighting)0;
	lighting.vsm_atlas    = vsm_lookup.vsm_atlas;
	lighting.page_table   = vsm_lookup.page_table;
	lighting.page_cameras = vsm_lookup.page_cameras;

	// get_shadow_vsm_simple returns a scalar (0/1 hardware-PCF average, no
	// transparency) -- broadcast to float3 to match sun_vis's own
	// transparent-aware float3 transmittance shape below.
	float shadow = get_shadow_vsm_simple(c, lighting, wpos, normal, light_dir);
	return float3(shadow, shadow, shadow);
}


// Real DXR any-hit: alpha-cutout support for ColorPass's general rays
// (reflections, GI, ...) and ColorShadowPass's sun-visibility rays. Unlike
// VSM's own RTX blocker-search verify ray (which fires from a compute
// shader via inline RayQuery and has no local root signature to reach a
// material's compiled shader with -- see VSM_ShadowResolve.hlsl's own
// comment), a real TraceRay()-driven hit group DOES have one, so this can
// evaluate the material's REAL compiled opacity via ShadowSurface() above
// (the same COMPILED_FUNC call MyClosestHitShader/ColorShadowClosestHitShader
// already make) instead of VSM's flat-texture-index workaround.
//
// IgnoreHit() below the same 0.5 threshold universal()'s clip(albedo.w-0.5)
// and VSM's own cutout paths already use, so a hole reads identically
// everywhere in the engine. Only ever actually invoked for instances built
// with D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE (MeshAssetInstance::
// update_rtx_instance(), set only for TransparencyMode::Masked) -- opaque
// geometry stays flagged opaque at the BLAS level and Translucent instances
// are FORCE_OPAQUE, so any-hit is skipped for them entirely by the hardware.
[shader("anyhit")]
void MyAnyHitShader(inout RayPayload payload, in MyAttributes attr)
{
	float4 color; float opacity, surface_transmission;
	ShadowSurface(attr, color, opacity, surface_transmission);

	if (opacity < 0.5)
		IgnoreHit();
}

[shader("anyhit")]
void ColorShadowAnyHitShader([raypayload] inout ColorShadowPayload payload, in MyAttributes attr)
{
	float4 color; float opacity, surface_transmission;
	ShadowSurface(attr, color, opacity, surface_transmission);

	if (opacity < 0.5)
		IgnoreHit();
}


// Transparent-aware shadow closest-hit (per material). Opaque materials block the
// light; transparent ones tint the transmittance by their colour/opacity and
// continue the ray toward the light through further layers.
[shader("closesthit")]
void ColorShadowClosestHitShader([raypayload] inout ColorShadowPayload payload, in MyAttributes attr)
{
	payload.dist = RayTCurrent(); // report the occluder distance to the caller.

#if defined(TRANSLUCENT)
	float4 color; float opacity, surface_transmission;
	ShadowSurface(attr, color, opacity, surface_transmission);

	payload.transmittance *= saturate(color.rgb) * saturate(surface_transmission);
#elif defined(TRANSPARENT)
	float4 color; float opacity, surface_transmission;
	ShadowSurface(attr, color, opacity, surface_transmission);

	// Light through the glass is tinted by its colour; opacity darkens it (full
	// opacity == opaque == blocks). The caller advances past and keeps tracing.
	payload.transmittance *= color.rgb * (1.0 - saturate(opacity));
#else
	payload.transmittance = 0.0; // opaque occluder -> full shadow
#endif
}



// TranslucentRTX's surface query: evaluate the material, hand the result back.
// All refraction/absorption logic lives in the raygen (rtx/translucency.hlsl).
[shader("closesthit")]
void TranslucentClosestHitShader([raypayload] inout TranslucentPayload payload, in MyAttributes attr)
{
	SceneData sceneData = CreateSceneData();
	RaytraceInstanceInfo instance = sceneData.GetRaytraceInstanceInfo()[InstanceID()];

	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

	uint id0 = instance.GetIndices()[PrimitiveIndex() * 3];
	uint id1 = instance.GetIndices()[PrimitiveIndex() * 3 + 1];
	uint id2 = instance.GetIndices()[PrimitiveIndex() * 3 + 2];

	Triangle t;
	t.init(instance.GetVertexes()[id0], instance.GetVertexes()[id1], instance.GetVertexes()[id2], barycentrics);
	t.v.pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

	float4 color = 1;
	float metallic = 0, roughness = 0, opacity = 1, refraction = 1, thickness = 0, transmission = 1, absorption_distance = 1;
	float4 normal = 0, glow = 0;
	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, opacity, refraction, thickness, transmission, absorption_distance, t.lod);

	// Inverse-transpose of the node's full local->world matrix (row-vector
	// multiply by the inverse), so non-uniform scale doesn't skew it either.
	node_data node = sceneData.GetNodes()[instance.GetNode_offset()];
	float3 world_normal = normalize(mul(t.v.normal, (float3x3)node.GetNode_inverse_matrix()));

	payload.dist        = RayTCurrent();
	payload.normal      = world_normal;
	payload.albedo      = color.rgb;
	payload.roughness   = roughness;
	payload.ior                 = refraction;
	payload.thickness           = thickness;
	payload.transmission        = transmission;
	payload.absorption_distance = absorption_distance;
	// From the interpolated normal rather than HitKind(): front-face winding
	// depends on the asset's triangle order, the normals are what the artist
	// actually authored as "outside".
	payload.front_face  = dot(WorldRayDirection(), world_normal) < 0 ? 1 : 0;
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
	float refraction = 1, thickness = 0, transmission = 1, absorption_distance = 1;


	t.lod += log2(abs(payload.cone.width + payload.cone.angle * RayTCurrent()));
	t.lod -= log2(abs(dot(normalize(WorldRayDirection()), t.v.normal)));


	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, opacity, refraction, thickness, transmission, absorption_distance, t.lod);

	// Transparent-aware sun visibility (float3 transmittance instead of a bool).
	// Iterate the shadow ray through transparent occluders (advancing past each
	// hit) until it hits something opaque (fully black) or misses (reaches the
	// light). The loop keeps every trace at the same recursion depth, so however
	// many glass layers there are it costs only one level. Gated at shallow color
	// recursion so deep refraction bounces don't spawn shadow rays.
	float3 sun_vis = 1.0;
	if (payload.use_vsm_shadow)
	{
		// Cheap path (RayPayload::use_vsm_shadow's own comment, raytracing.prism):
		// one VSM lookup instead of a real recursive shadow ray -- no
		// transparent-occluder iteration (VSM's own shadow atlas doesn't
		// carry per-material transmittance the way ColorShadowPass does),
		// but DDGI's own probe texels don't need that precision.
		sun_vis = vsm_shadow_lookup(t.v.pos, t.v.normal, normalize(frame.GetSunDir().xyz));
	}
	else if (payload.recursion < 2)
	{
		float3 s_origin = t.v.pos;
		float3 s_dir    = normalize(frame.GetSunDir().xyz);

		[loop]
		for (int si = 0; si < 1; si++)
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
	// Self-emission: additive, not shadowed or BRDF-weighted -- same
	// convention as rtx_combine.hlsl's own direct-visibility glow term.
	// This is what makes indirect GI bounces and reflection rays pick up
	// glowing surfaces at all: both just forward this payload.color through
	// denoise + combine (rtx_combine.hlsl / reflection_combine.hlsl), which
	// weight it as arriving radiance without caring where it came from.
	payload.color = float4(color.rgb * NdotL * sun_vis * (1 - metallic) + glow.rgb, 1.0);
	payload.dist  = RayTCurrent();
	// DDGIProbeTrace's own feedback-term consumer -- see this payload
	// field's own comment (raytracing.prism) for why these two exist.
	payload.hit_normal = t.v.normal;
	payload.albedo      = color.rgb * (1 - metallic);

//#ifdef TRANSPARENT
//	// Transparent surface: continue a refracted color ray through the surface
//	// (Snell's law with the material's refraction index) and blend it behind the
//	// surface shading according to opacity. Guarded by recursion depth.
//	if (payload.recursion < 6)
//	{
//		float3 wi = normalize(WorldRayDirection());
//		float3 N  = normalize(t.v.normal);

//		// Orient the normal against the incoming ray. entering = hitting the outer
//		// (air-side) face; !entering = the ray is leaving the medium (back face).
//		bool entering = dot(wi, N) < 0.0;
//		if (!entering) N = -N;

//		// eta = n_from / n_to.
//		float eta = entering ? (1.0 / refraction) : refraction;

//		// Fresnel (Schlick) reflectance for the air/medium interface: grazing
//		// angles reflect more, near-normal angles transmit more.
//		float R0   = (refraction - 1.0) / (refraction + 1.0);
//		R0        *= R0;
//		float cosI = saturate(dot(-wi, N));
//		float F    = R0 + (1.0 - R0) * pow(1.0 - cosI, 5.0);

//		float3 refr_dir = refract(wi, N, eta);
//		bool   tir      = dot(refr_dir, refr_dir) < 1e-4;
//		if (tir) F = 1.0; // total internal reflection: everything reflects

//		// Beer-Lambert absorption coefficient from the glass tint.
//		const float absorption = 0.35; // tune: higher = tints faster with distance
//		float3 sigma = (1.0 - saturate(color.rgb)) * absorption;

//		float3 reflected = 0;
//		float3 refracted = 0;

//		// Reflection ray. Only branch at shallow recursion to keep the ray tree
//		// from exploding; deeper bounces transmit only.
//		if (F > 0.01 && payload.recursion < 3)
//		{
//			[raypayload] RayPayload rp;
//			rp.init();
//			rp.recursion = payload.recursion + 1;
//			rp.cone      = payload.cone;

//			RayDesc rr;
//			rr.Origin    = t.v.pos;
//			rr.Direction = reflect(wi, N);
//			rr.TMin      = 0.001;
//			rr.TMax      = 10000.0;
//			ColorPass(raytracing.GetScene(), rr, RAY_FLAG_NONE, rp);

//			reflected = rp.color.rgb;
//			// A reflection off the inner boundary stays inside the medium.
//			if (!entering) reflected *= exp(-sigma * rp.dist);
//		}

//		// Refraction ray (skipped on TIR or when its Fresnel weight is negligible).
//		if (!tir && F < 0.99)
//		{
//			[raypayload] RayPayload rp;
//			rp.init();
//			rp.recursion = payload.recursion + 1;
//			rp.cone      = payload.cone;

//			RayDesc rr;
//			rr.Origin    = t.v.pos;
//			rr.Direction = refr_dir;
//			rr.TMin      = 0.001;
//			rr.TMax      = 10000.0;
//			ColorPass(raytracing.GetScene(), rr, RAY_FLAG_NONE, rp);

//			refracted = rp.color.rgb;
//			// Refracting inward travels through the medium -> Beer-Lambert absorb.
//			if (entering) refracted *= exp(-sigma * rp.dist);
//		}

//		// Fresnel blend of reflection and transmission, then surface coverage.
//		float3 combined = lerp(refracted, reflected, F);
//		payload.color = lerp(float4(combined, 1.0), payload.color, saturate(opacity));
//	}
//#endif
}