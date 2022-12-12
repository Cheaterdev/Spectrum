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

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f, out float4 g, float lod);



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


	t.lod += log2(abs(payload.cone.width + payload.cone.angle * RayTCurrent()));
	t.lod -= log2(abs(dot(normalize(WorldRayDirection()), t.v.normal)));


	COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, t.lod);


	
	float3 lightDir = frame.GetSunDir();
	RayPayload payload2 = payload.propagate();

	float shadow = 1;
	if (payload2.recursion <= 1)
	{


		float hit_rate = 0;
		int samples = 1;// payload2.recursion < 2 ? 3 : 1;
		for (int i = 0; i < samples; i++)
		{
			float3 dir = GetRandomDir(t.v.tc, lightDir, 0, i);

			ShadowPayload payload_shadow = { false };

			RayDesc ray;
			ray.Origin = t.v.pos;
			ray.Direction = dir;
			ray.TMin = 0.00001;
			ray.TMax = 10000.0;
			ShadowPass(raytracing.GetScene(), ray, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, payload_shadow);

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

	float3 my_color = color * max(0.01, (shadow)*saturate(dot(t.v.normal, lightDir)));



	payload.color = float4(my_color,1);// float4(lerp(my_color, reflected, fresnel), 1);


	float3 voxel_min = CreateVoxelInfo().GetMin().xyz;
	float3 voxel_size = CreateVoxelInfo().GetSize().xyz;

	//float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14);

//	float angle_coeff = saturate(max_angle / (angle + 0.01));
	//angle = min(angle, max_angle);
	float3 origin = saturate(((t.v.pos - (voxel_min)) / voxel_size));

//	float4 voxel = get_voxel(origin,0);
//	payload.color = float4(voxel.xyz/ (voxel.w+0.001) + glow, 1);


	payload.dist = RayTCurrent();
}