#include "autogen/FrameInfo.h"
#include "autogen/MaterialInfo.h"
#include "autogen/SceneData.h"
#include "autogen/Raytracing.h"

#define Sampler linearSampler
#define GetMaterialInfo CreateMaterialInfo

#define REFRACTION
struct vertex_input
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 tc : TEXCOORD;
	float4 tangent : TANGENT;
	//float3 binormal : BINORMAL;
};



typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
	float4 color;
	uint recursion;
	float dist;
};


struct ShadowPayload
{
	bool hit;
};


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
#define sample(tex, s,  tc) tex.Sample(s, tc);

#else
#define sample(tex, s,  tc)  tex.SampleLevel(s, tc, 0);

#endif 

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f);


float calc_fresnel(float k0, float3 n, float3 v)
{
	float ndv = saturate(dot(n, -v));
	return k0 + (1 - k0) * pow(1 - ndv, 5);
	return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{

	SceneData sceneData = CreateSceneData();
	Raytracing raytracing = CreateRaytracing();

	MeshInstance instance = sceneData.GetMeshInstances()[InstanceID()];

	uint vertex_offset = instance.GetVertex_offset();
	uint index_offset = instance.GetIndex_offset();

	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

//	InstanceData instance = instances[InstanceID() - 1];

	uint id0 = raytracing.GetIndex_buffer()[index_offset + PrimitiveIndex() * 3];
	uint id1 = raytracing.GetIndex_buffer()[index_offset + PrimitiveIndex() * 3 + 1];
	uint id2 = raytracing.GetIndex_buffer()[index_offset + PrimitiveIndex() * 3 + 2];

	vertex_input vertex0 = sceneData.GetVertexes()[vertex_offset + id0];
	vertex_input vertex1 = sceneData.GetVertexes()[vertex_offset + id1];
	vertex_input vertex2 = sceneData.GetVertexes()[vertex_offset + id2];

	vertex_input vertex;
	
	vertex.normal = ((HitKind()== HIT_KIND_TRIANGLE_FRONT_FACE)*2-1)*(vertex0.normal * barycentrics.x + vertex1.normal * barycentrics.y + vertex2.normal * barycentrics.z);
	vertex.tc = vertex0.tc * barycentrics.x + vertex1.tc * barycentrics.y + vertex2.tc * barycentrics.z;

	float3 pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
	//float3 view = -normalize(camera.position - pos);


	float kR = 0.9;
	float3 refl = reflect(WorldRayDirection(), vertex.normal);
	float3 refr = refract(WorldRayDirection(), vertex.normal, HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE? kR :(1.0/ kR));
	float4 color = 1;
	float metallic = 1;
	float roughness = 1;
	float4 normal = 0;


	COMPILED_FUNC(pos, vertex.tc, color, metallic, roughness, normal);
	
	float3 lightDir = normalize(float3(0, 1, 1));
	RayPayload payload2 = { float4(0.0, 0.0, 0.0, 1) , payload.recursion + 1 ,0};
	ShadowPayload payload_shadow = { false };

	if (payload2.recursion < 5)
	{
		
		{
			RayDesc ray;
		ray.Origin = pos;
		ray.Direction = normalize(refl);
		ray.TMin = 0.01;
		ray.TMax = 1000.0;
		TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload2);
		}






		{
			RayDesc ray;
			ray.Origin = pos;
			ray.Direction = normalize(float3(0, 1, 1));
			ray.TMin = 0.001;
			ray.TMax = 1000.0;
			TraceRay(raytracing.GetScene(), RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 1, 0, 1, ray, payload_shadow);
		}
	}

#ifdef REFRACTION

	if (payload2.recursion < 5)
	{

		RayPayload payload_refraction = { float4(0.0, 0.0, 0.0, 0) , payload.recursion + 1 ,0};

		{
			RayDesc ray;
			ray.Origin = pos;
			ray.Direction = normalize(refr);
			ray.TMin = 0.01;
			ray.TMax = 1000.0;
			TraceRay(raytracing.GetScene(), RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload_refraction);
		}


		if (HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE)
		{
			color = color* payload_refraction.color;// lerp(color, 1, exp(-payload_refraction.dist / 1)) * payload_refraction.color;
		//	color= exp(-payload_refraction.dist / 1);
		}
		else
		{
			color =  payload_refraction.color;
		}

		//

	}
	else
		color = 0;


	float3 my_color = color;// +10 * pow(max(0, dot(refl, lightDir)), 256);

#else
	float3 my_color = color *max(0.01, (!payload_shadow.hit) * dot(vertex.normal, lightDir));

#endif

	float fresnel =  calc_fresnel(roughness, vertex.normal, WorldRayDirection());

	
	float3 reflected = payload2.color.xyz;
	payload.color = float4(lerp(my_color, reflected, fresnel), 1);

	payload.dist = RayTCurrent();
}