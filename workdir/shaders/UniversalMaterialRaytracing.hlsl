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


	//COMPILED_FUNC(t.v.pos, t.v.tc, color, metallic, roughness, normal, glow, t.lod);


	
	// debug: visualize world-space normals — if sphere appears, TLAS + hit shader work
	payload.color = float4(abs(t.v.normal), 1.0);
	payload.dist  = RayTCurrent();
}