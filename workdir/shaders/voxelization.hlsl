
#include "autogen/Voxelization.h"

static const VoxelInfo voxel_info = GetVoxelization().GetInfo();

struct vertex_output
{
float4 pos : SV_POSITION;
float3 wpos : WPOSITION;
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
	return float4((In.xyz-voxel_info.GetMin().xyz)/voxel_info.GetSize().xyz,1);
}
float4 TransformPosition(float2 In)
{
	return float4((In)*2-1,0,1);
}

[maxvertexcount(3)]
void GS( triangle vertex_output I[3], inout TriangleStream<vertex_output> triStream )
{
	
	I[0].prev_pos=TransformPos(I[0].wpos);
	I[1].prev_pos=TransformPos(I[1].wpos);
	I[2].prev_pos=TransformPos(I[2].wpos);


	float3 Delta1=I[0].prev_pos.xyz-I[1].prev_pos.xyz;
	float3 Delta2=I[0].prev_pos.xyz-I[2].prev_pos.xyz;

	float S1=length(cross(float3(Delta1.xy,0),float3(Delta2.xy,0)));
	float S2=length(cross(float3(Delta1.xz,0),float3(Delta2.xz,0)));
	float S3=length(cross(float3(Delta1.yz,0),float3(Delta2.yz,0)));

			
	
	if(S1>S2){
		if(S1>S3)
		{
			I[0].pos=TransformPosition(I[0].prev_pos.xy);
			I[2].pos=TransformPosition(I[1].prev_pos.xy);
			I[1].pos=TransformPosition(I[2].prev_pos.xy);
		}else{
			I[0].pos=TransformPosition(I[0].prev_pos.yz);
			I[1].pos=TransformPosition(I[1].prev_pos.yz);
			I[2].pos=TransformPosition(I[2].prev_pos.yz);
		}
	}
	else
	{
		if(S2>S3)
		{
			I[0].pos=TransformPosition(I[0].prev_pos.xz);
			I[1].pos=TransformPosition(I[1].prev_pos.xz);
			I[2].pos=TransformPosition(I[2].prev_pos.xz);
		}else{
			I[0].pos=TransformPosition(I[0].prev_pos.yz);
			I[1].pos=TransformPosition(I[1].prev_pos.yz);
			I[2].pos=TransformPosition(I[2].prev_pos.yz);
		}
	}

	triStream.Append(I[0]);
	triStream.Append(I[1]);
	triStream.Append(I[2]);
	triStream.RestartStrip();
}
