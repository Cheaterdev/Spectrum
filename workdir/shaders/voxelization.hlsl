
#include "autogen/Voxelization.h"

static const VoxelInfo voxel_info = GetVoxelization().GetInfo();

struct vertex_output
{
	float4 pos : SV_POSITION;
	float3 wpos : POSITION;
	float3 normal : NORMAL;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float2 tc : TEXCOORD;
	float4 cur_pos : CUR_POSITION;

	float4 prev_pos : PREV_POSITION;
	float dist : DISTANCE;
};


float4 TransformPos(float3 In)
{
	return float4((In.xyz - voxel_info.GetMin().xyz) / voxel_info.GetSize().xyz, 1);
}
float4 TransformPosition(float2 In)
{
	return float4((In) * 2 - 1, 0, 1);
}

[maxvertexcount(3)]
void GS(in triangle vertex_output I[3], inout TriangleStream<vertex_output> triStream)
{
	float3 p0 = TransformPos(I[0].wpos).xyz;
	float3 p1 = TransformPos(I[1].wpos).xyz;
	float3 p2 = TransformPos(I[2].wpos).xyz;


	float3 Delta1 = p0 - p1;
	float3 Delta2 = p0 - p2;

	float S1 = abs(cross(float3(Delta1.xy, 0), float3(Delta2.xy, 0)).z);
	float S2 = abs(cross(float3(Delta1.xz, 0), float3(Delta2.xz, 0)).z);
	float S3 = abs(cross(float3(Delta1.yz, 0), float3(Delta2.yz, 0)).z);



	if (S1 > S2 && S1 > S3)
	{
		I[0].pos = TransformPosition(p0.xy);
		I[1].pos = TransformPosition(p1.xy);
		I[2].pos = TransformPosition(p2.xy);
	}
	else if (S2 > S1 && S2 > S3)
	{
		I[0].pos = TransformPosition(p0.xz);
		I[1].pos = TransformPosition(p1.xz);
		I[2].pos = TransformPosition(p2.xz);
	}
	else
	{
		I[0].pos = TransformPosition(p0.yz);
		I[1].pos = TransformPosition(p1.yz);
		I[2].pos = TransformPosition(p1.yz);
	}


	triStream.Append(I[0]);
	triStream.Append(I[1]);
	triStream.Append(I[2]);
	triStream.RestartStrip();
}
