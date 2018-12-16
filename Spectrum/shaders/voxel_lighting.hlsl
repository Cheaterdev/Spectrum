//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard
//
#include "Common.hlsl"

RWTexture3D<float4> output : register(u0);
Texture3D<float4> tex_albedo : register(t0);
Texture3D<float4> tex_normals : register(t1);
Texture3D<float4> tex_lower : register(t2);

SamplerState BilinearClamp : register(s0);
SamplerComparisonState cmp_sampler: register(s1);
SamplerState LinearSamplerClamp : register(s2);

#define SINGLE_SAMPLE

//#include "PSSM_impl.hlsl"

Texture2D<float> light_buffer: register(t4);
StructuredBuffer<camera_info> light_cameras : register(t5);

float get_shadow(float3 wpos)
{
	uint4 shadow_dims = uint4(512, 512, 512, 512);
	light_buffer.GetDimensions(shadow_dims.x, shadow_dims.y);
	int level =0 ;
	camera_info light_cam = light_cameras[level];
	float4 pos_l = mul(light_cam.view_proj, float4(wpos , 1));
	pos_l.z -= 0.001 * pow(2, level);
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	float shadow = light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z);

	if (pos_l.z < 0 || pos_l.x > 1 || pos_l.x < -1 || pos_l.y > 1 || pos_l.y < -1)
		shadow = 1;

	return shadow;
}




TextureCube<float4> tex_cube : register(t6);
StructuredBuffer<int3> voxel_visibility: register(t7);



float4 get_sky(float3 dir, float level)
{

	//return 0;
	level *= 256;
	//return tex_cube.Sample(LinearSampler, float4(dir,6));
	int l0 = level;
	int l1 = l0 + 1;


	return tex_cube.SampleLevel(BilinearClamp, float4(dir, l0), 0);


}
//#define voxel_min float3(-150,-150,-150)
//#define voxel_size float3(300,300,300)

float4 get_voxel(float3 pos, float level)
{
return tex_lower.SampleLevel(BilinearClamp,pos,level);
}

float get_shadow_voxel(float3 pos)
{
return tex_albedo.SampleLevel(BilinearClamp,pos,0).w>0;
}

float trace_shadow(float3 origin,float3 dir)
{
 
	dir=normalize(dir);
	 float3 samplePos = 0;
    float accum = 0;
    // the starting sample diameter
    float minDiameter = 1.0 / 512;
    float minVoxelDiameterInv = 1.0 / minDiameter;
    // push out the starting point to avoid self-intersection
    float startDist = (1) * minDiameter;
   
    float maxDist = 1;
    float dist = startDist;

    while (dist <= maxDist && accum < 1.0 && all(samplePos <= 1) && all(samplePos >= 0))
    {

        samplePos = origin + dir * dist;
        float sampleValue = get_shadow_voxel(samplePos);
	
        float sampleWeight = (1 - accum);
        accum += sampleValue * sampleWeight;
        dist += minDiameter;
    }

		return accum;
}


float4 trace(float3 origin, float3 dir, float3 normal, float angle)
{
	dir = normalize(dir);

	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14);// *1.3;
	float angle_coeff = saturate(max_angle / (angle + 0.01));

	//angle = min(angle, max_angle);
 
		 float3 samplePos = 0;
    float4 accum = 0;
    // the starting sample diameter
    float minDiameter = 1.0 / 256;
    float minVoxelDiameterInv = 1.0 / minDiameter;
    // push out the starting point to avoid self-intersection
  //  float startDist = (1) * minDiameter;
   
    float maxDist = 1;
    float dist = minDiameter;

    while (dist <= maxDist && accum.w < 0.6 && all(samplePos <= 1) && all(samplePos >= 0))
    {
	    float sampleDiameter = max(minDiameter, angle * dist);
    
       float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
        samplePos = origin + dir * dist;
        float4 sampleValue = get_voxel(samplePos, sampleLOD);
	   float sampleWeight = (1 - accum.w);
        accum += sampleValue * sampleWeight;
        dist += sampleDiameter;
    }

//	accum.xyz *= angle_coeff;

	//float4 sky = get_sky(dir,angle);
 // float sampleWeight = saturate(1 - accum.w*2);
   //  accum += sky * pow(sampleWeight,1);

		return accum;
	


}

float4 get_direction(float3 pos, float3 normal, float3 dir, float k, float a)
{

	return trace(pos, dir, normal, a);//*pow(dot(normal, dir), 8);
}
float4 getGI(float3 Pos, float3 Normal)
{
	float a = 0.6;

	float4 Color = 0;
	float t = 0.4;
	float k = 0;
	float3 right = t*normalize(cross(Normal, float3(0, 1, 0.001)));
	float3 tangent = t*normalize(cross(right, Normal));

	Color += get_direction(Pos, Normal, normalize(Normal + right), k, a);// trace(Pos + k*normalize(Normal + right), normalize(Normal + right), a);
	Color += get_direction(Pos, Normal, normalize(Normal - right), k, a);//trace(Pos + k*normalize(Normal - right), normalize(Normal - right), a);
	Color += get_direction(Pos, Normal, normalize(Normal + tangent), k, a);//trace(Pos + k*normalize(Normal + tangent), normalize(Normal + tangent), a);
	Color += get_direction(Pos, Normal, normalize(Normal - tangent), k, a);//trace(Pos + k*normalize(Normal - tangent), normalize(Normal - tangent), a);
	Color += get_direction(Pos, Normal, normalize(Normal + tangent - right), k, a);//trace(Pos + k*normalize(Normal + tangent - right), normalize(Normal + tangent - right), a);
	Color += get_direction(Pos, Normal, normalize(Normal + tangent + right), k, a);//trace(Pos + k*normalize(Normal + tangent + right), normalize(Normal + tangent + right), a);
	Color += get_direction(Pos, Normal, normalize(Normal - tangent - right), k, a);//trace(Pos + k*normalize(Normal - tangent - right), normalize(Normal - tangent - right), a);
	Color += get_direction(Pos, Normal, normalize(Normal - tangent + right), k, a);//trace(Pos + k*normalize(Normal - tangent + right), normalize(Normal - tangent + right), a);
																				   //  Color /= 3.14f;
																				   //Color *= getAO(Pos-0*Normal, Normal);
	return  Color /8;

}


/*
cbuffer CB : register(b0)
{
    uint SrcMipLevel;	// Texture level of source mip
    uint NumMipLevels;	// Number of OutMips to write: [1, 4]
    float2 TexelSize;	// 1.0 / OutMip1.Dimensions
}
*/

//#define voxel_min float3(-150,-150,-150)
//#define voxel_size float3(300,300,300)

cbuffer consts:register(b0)
{
	float3 dir; 
	float3 voxel_min;
	float3 voxel_size;
			//ivec3 voxel_map_size; int groups;

	uint4 voxels_per_tile;

	
};



static const float scaler = voxel_size / 512;

//static const float3 voxel_min=float3(voxel_min_x,voxel_min_y,voxel_min_z);
//static const float3 voxel_size=float3(voxel_size_x,voxel_size_y,voxel_size_z);

uint3 get_index( uint3 groupThreadID,	uint3 groupID   )
{
	
	uint3 tile_index = voxel_visibility[groupID.x/voxels_per_tile.w];
	int tile_local_index = groupID.x%voxels_per_tile.w;
	uint3 index = tile_index*voxels_per_tile.xyz + groupThreadID + 4 * int3(tile_local_index % 8, (tile_local_index / 8) % 4, tile_local_index / (4 * 8));//groupID*4+groupThreadID;
	return index;

}
[numthreads(4, 4, 4)]
void CS(    
	uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex)
{

uint3 index = get_index(groupThreadID, groupID);
	
   float3 dims;
   output.GetDimensions(dims.x, dims.y, dims.z);
   
	if(any(index>dims)||any(index<0)) return;
	
	float4 albedo = tex_albedo[index];//SrcMip.SampleLevel(BilinearClamp,2*index/dims,0);
	
//	output[index] = albedo;
	//return;
	//output[index] = 1;

	//return;
	if(albedo.w==0)
	{
		output[index] = 0;
		return;
	}
	

	float3 normals = normalize(tex_normals[index].xyz*2-1);
	
	float m = max(max(abs(normals.x),abs(normals.y)),abs(normals.z));
#ifdef SECOND_BOUNCE
	float4 gi = getGI((index + normals / m) / dims, normals);
#else
	float4 gi = 0;
#endif
	float3 pos = index*voxel_size/dims+voxel_min+ scaler*normals/m;
	
//	float traced_shadow = (1-trace_shadow((index+1*normals)/dims,dir));
	float shadow = saturate(dot(normals,dir))*get_shadow(pos);
	
	
	float3 lighting =albedo.xyz*gi.xyz + 1 * albedo.xyz*shadow;//saturate(dot(normals,float3(0,1,0)));
//	output[index] = float4(albedo.xyz,albedo.w);
	//lighting = traced_shadow.xxx	;
	output[index] = float4(lighting,1);
}
