#include "autogen/FrameInfo.h"
#include "autogen/MaterialInfo.h"
#include "autogen/SceneData.h"
#include "autogen/Raytracing.h"

#include "autogen/tables/RayPayload.h"
#include "autogen/tables/ShadowPayload.h"

#include "autogen/tables/Triangle.h"

//#define REFRACTION
#define Sampler linearSampler
#define GetMaterialInfo CreateMaterialInfo

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}


Texture2D get_texture(uint i)
{
	return CreateSceneData().GetMaterial_textures(i + CreateMaterialInfo().GetTextureOffset());

	//	return textures[i /*+ texture_offset*/];
}

#ifdef BUILD_FUNC_PS
#define sample(tex, s,  tc, lod) tex.Sample(s, tc);
#else
float4 sample(Texture2D tex, SamplerState s, float2 tc, float lod)
{
	uint2 size;
	tex.GetDimensions(size.x, size.y);
	lod += 0.5 * log2(size.x * size.y);

	return tex.SampleLevel(s, tc, lod);
}
#endif 

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f, float lod);


float calc_fresnel(float k0, float3 n, float3 v)
{
	float ndv = saturate(dot(n, -v));
	return k0 + (1 - k0) * pow(1 - ndv, 5);
	return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}


[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{


	FrameInfo frame = CreateFrameInfo();
	SceneData sceneData = CreateSceneData();
	Raytracing raytracing = CreateRaytracing();

	MeshInstance instance = sceneData.GetMeshInstances()[InstanceID()];

	uint vertex_offset = instance.GetVertex_offset();
	uint index_offset = instance.GetIndex_offset();

	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);


	uint id0 = raytracing.GetIndex_buffer()[index_offset + PrimitiveIndex() * 3];
	uint id1 = raytracing.GetIndex_buffer()[index_offset + PrimitiveIndex() * 3 + 1];
	uint id2 = raytracing.GetIndex_buffer()[index_offset + PrimitiveIndex() * 3 + 2];

	mesh_vertex_input vertex0 = sceneData.GetVertexes()[vertex_offset + id0];
	mesh_vertex_input vertex1 = sceneData.GetVertexes()[vertex_offset + id1];
	mesh_vertex_input vertex2 = sceneData.GetVertexes()[vertex_offset + id2];


	Triangle t;
	t.init(vertex0, vertex1, vertex2, barycentrics);

	if (HitKind() != HIT_KIND_TRIANGLE_FRONT_FACE)
		t.v.normal = -t.v.normal;

	t.v.pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

	float kR = 0.9;
	float3 refl = reflect(WorldRayDirection(), t.v.normal);
	float3 refr = refract(WorldRayDirection(), t.v.normal, HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? kR : (1.0 / kR));
	float4 color = 1;
	float metallic = 1;
	float roughness = 1;
	float4 normal = 0;


	t.lod += log2(abs(payload.cone.width + payload.cone.angle * RayTCurrent()));
	t.lod -= log2(abs(dot(normalize(WorldRayDirection()), t.v.normal)));


	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, t.lod);


	
	float3 lightDir = frame.GetSunDir();
	RayPayload payload2 = payload.propagate();

	float shadow = 1;
	if (payload2.recursion <= 1)
	{


		float hit_rate = 0;
		int samples = 1;// payload2.recursion < 2 ? 3 : 1;
		for (int i = 0; i < samples; i++)
		{
			float time = 0;// frame.GetTime().y + i;
			float sini = sin(time * 220 + float(t.v.tc.x));
			float cosi = cos(time * 220 + float(t.v.tc.y));
			float rand = rnd(float2(sini, cosi));


			float rcos = cos(6.14 * rand);
			float rsin = sin(6.14 * rand);
			float rand2 = rnd(float2(cosi, sini));

			float tt = 0;// 0.1 * pow(rand2, 1.0 / 3.0);

			float3 right = rsin * tt * normalize(cross(lightDir, float3(0, 1, 0.1)));
			float3 tangent = rcos * tt * normalize(cross(right, lightDir));

			float3 dir = normalize(lightDir + right + tangent);



			ShadowPayload payload_shadow = { false };

			RayDesc ray;
			ray.Origin = t.v.pos;
			ray.Direction = dir;
			ray.TMin = 0.003;
			ray.TMax = 1000.0;
			TraceRay(raytracing.GetScene(), RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 1, 0, 1, ray, payload_shadow);

			if (payload_shadow.hit)
				hit_rate += 1.0f;

		}
		shadow = 1.0 - hit_rate / samples;
	}
	/*
#ifdef REFRACTION

	if (payload2.recursion < 5)
	{

		RayPayload payload_refraction = payload.propagate();

		{
			RayDesc ray;
			ray.Origin = t.v.pos;
			ray.Direction = normalize(refr);
			ray.TMin = 0.01;
			ray.TMax = 1000.0;
			TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload_refraction);
		}


		if (HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE)
		{
			color = color * payload_refraction.color;// lerp(color, 1, exp(-payload_refraction.dist / 1)) * payload_refraction.color;
		//	color= exp(-payload_refraction.dist / 1);
		}
		else
		{
			color = payload_refraction.color;
		}

		//

	}
	else
		color = 0;


	float3 my_color = color;// +10 * pow(max(0, dot(refl, lightDir)), 256);

#else
	float3 my_color = color * max(0.01, (shadow)*saturate(dot(t.v.normal, lightDir)));

#endif

	float fresnel = calc_fresnel(roughness, t.v.normal, WorldRayDirection());


	float3 reflected = payload2.color.xyz;*/
	payload.color = float4(shadow* color.xyz,1);// float4(lerp(my_color, reflected, fresnel), 1);

	payload.dist = RayTCurrent();
}