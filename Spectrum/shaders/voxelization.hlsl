
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


//#define voxel_min float3(-50,-50,-50)
//#define voxel_size float3(100,100,100)
/*cbuffer voxel_info : register(b3)
{
   //	float3 voxel_min:packoffset(c0);
	//float3 voxel_size:packoffset(c3);
	
	float voxel_min_x;
	float voxel_min_y;
	float voxel_min_z;
	float voxel_size_x;
	float voxel_size_y;
	float voxel_size_z;
	
}; 
static const float3 voxel_min=float3(voxel_min_x,voxel_min_y,voxel_min_z);
static const float3 voxel_size=float3(voxel_size_x,voxel_size_y,voxel_size_z);
*/

 

struct voxel_info_struct 
{
	float4 voxel_min;
	float4 voxel_size;
	int4 voxel_map_size; 
};
ConstantBuffer<voxel_info_struct> voxel_info: register(b3);
float4 TransformPos(float3 In)
{
	return float4((In.xyz-voxel_info.voxel_min.xyz)/voxel_info.voxel_size.xyz,1);
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
