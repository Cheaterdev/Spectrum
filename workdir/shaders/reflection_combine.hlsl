
#include "Common.hlsl"


#include "autogen/FrameInfo.h"
#include "autogen/ReflectionCombine.h"


static const Camera camera = GetFrameInfo().GetCamera();
static const GBuffer gbuffer = GetReflectionCombine().GetGbuffer();

float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, 0.5 + 0.5 * NoV), 0);
}


float3 get_PBR(float3 SpecularColor, float3 ReflectionColor, float3 N, float3 V, float Roughness, float Metallic)
{
	V *= -1;
	float NoV = dot(N, V);
	//return NoV<0;
//	float3 R = 2 * dot(V, N) * N - V;
//	float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
	float2 EnvBRDF = IntegrateBRDF(Roughness, Metallic, NoV);
	return     ReflectionColor * (Metallic * SpecularColor * EnvBRDF.x + EnvBRDF.y);
}


[numthreads(8,8,1)]
void CS(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{
		float2 dims;
	GetReflectionCombine().GetTarget().GetDimensions(dims.x, dims.y);

	int2 tc = dispatchID.xy;

	float2 itc = float2(tc + 0.5) / dims;
	
	float4 albedo = gbuffer.GetAlbedo()[tc];

	float  roughness = pow(max(MIN_ROUGHNESS, gbuffer.GetNormals()[tc].w), 2);
	float metallic = albedo.w;// specular.w;

	float raw_z = gbuffer.GetDepth()[tc.xy];
	float3 pos = depth_to_wpos(raw_z, itc, camera.GetInvViewProj());
	float3 v = normalize(pos - camera.GetPosition());

	float4 res = 0;
	float w = 0;
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);



	float3 reflection = GetReflectionCombine().GetReflection()[dispatchID.xy];;

	float3 color = get_PBR(albedo, reflection.xyz, normal, v, roughness, metallic);
//	output[index] = float4(lighting, 1);
	GetReflectionCombine().GetTarget()[dispatchID.xy]+=float4(color,0);
 //
}
