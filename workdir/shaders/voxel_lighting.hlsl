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


//#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelLighting.h"

static const VoxelInfo voxel_info = GetVoxelInfo();
static const RWTexture3D<float4> output = GetVoxelLighting().GetOutput();
static const Texture3D<float4> tex_albedo = GetVoxelLighting().GetAlbedo();
static const Texture3D<float4> tex_normals = GetVoxelLighting().GetNormals();
static const Texture3D<float4> tex_lower = GetVoxelLighting().GetLower();
static const TextureCube<float4> tex_cube = GetVoxelLighting().GetTex_cube();

static const float3 voxel_min = voxel_info.GetMin().xyz;
static const float3 voxel_size = voxel_info.GetSize().xyz;

static const Camera light_cam = GetVoxelLighting().GetPssmGlobal().GetLight_camera()[0];
static const float3 dir = -normalize(light_cam.GetDirection().xyz);
#define SINGLE_SAMPLE


float get_shadow(float3 wpos)
{
	float4 pos_l = mul(light_cam.GetViewProj(), float4(wpos, 1));
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	float shadow = GetVoxelLighting().GetPssmGlobal().GetLight_buffer().SampleLevel(linearSampler, light_tc, 0) > pos_l.z;
	return shadow;
}

float4 get_sky(float3 dir, float level)
{
	level *= 256;
	return tex_cube.SampleLevel(linearSampler, dir, level);
}

float4 get_voxel(float3 pos, float level)
{
	return float4(1, 1, 1, 1) * tex_lower.SampleLevel(linearClampSampler, pos, level);
}

float get_shadow_voxel(float3 pos)
{
	return tex_albedo.SampleLevel(linearClampSampler, pos, 0).w > 0;
}


float4 trace(float3 origin, float3 dir, float3 normal, float angle)
{
	dir = normalize(dir);

	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14);// *1.3;
	float angle_coeff = saturate(max_angle / (angle + 0.01));

	//angle = min(angle, max_angle);

	//origin += dir / 128;
	float3 samplePos = 0;
	float4 accum = 0;
	// the starting sample diameter
	float minDiameter = 1.0 / 256;
	float minVoxelDiameterInv = 1.0 / minDiameter;
	// push out the starting point to avoid self-intersection
  //  float startDist = (1) * minDiameter;

	float maxDist = 1;
	float dist = minDiameter;

	while (dist <= maxDist && accum.w < 1 && all(samplePos <= 1) && all(samplePos >= 0))
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

	float4 sky = get_sky(dir, angle);
	float sampleWeight = saturate(1 - accum.w);
	accum += sky * pow(sampleWeight, 1);

	return accum;



}

float4 get_direction(float3 pos, float3 normal, float3 dir, float k, float a)
{

	return trace(pos, dir, normal, a);// *pow(dot(normal, dir), 1);
}
float4 getGI(float3 Pos, float3 Normal)
{
	float a = 0.8;

	float4 Color = 0;
	float t = 1;
	float k = 0;
	float3 right = t * normalize(cross(Normal, float3(0, 1, 0.001)));
	float3 tangent = t * normalize(cross(right, Normal));

	Color += get_direction(Pos, Normal, normalize(Normal), k, a);// trace(Pos + k*normalize(Normal + right), normalize(Normal + right), a);
	return Color;


/*	Color += get_direction(Pos, Normal, normalize(Normal + right), k, a);// trace(Pos + k*normalize(Normal + right), normalize(Normal + right), a);
	Color += get_direction(Pos, Normal, normalize(Normal - right), k, a);//trace(Pos + k*normalize(Normal - right), normalize(Normal - right), a);
	Color += get_direction(Pos, Normal, normalize(Normal + tangent), k, a);//trace(Pos + k*normalize(Normal + tangent), normalize(Normal + tangent), a);
	Color += get_direction(Pos, Normal, normalize(Normal - tangent), k, a);//trace(Pos + k*normalize(Normal - tangent), normalize(Normal - tangent), a);
	Color += get_direction(Pos, Normal, normalize(Normal + tangent - right), k, a);//trace(Pos + k*normalize(Normal + tangent - right), normalize(Normal + tangent - right), a);
	Color += get_direction(Pos, Normal, normalize(Normal + tangent + right), k, a);//trace(Pos + k*normalize(Normal + tangent + right), normalize(Normal + tangent + right), a);
	Color += get_direction(Pos, Normal, normalize(Normal - tangent - right), k, a);//trace(Pos + k*normalize(Normal - tangent - right), normalize(Normal - tangent - right), a);
	Color += get_direction(Pos, Normal, normalize(Normal - tangent + right), k, a);//trace(Pos + k*normalize(Normal - tangent + right), normalize(Normal - tangent + right), a);
																				   //  Color /= 3.14f;
																				   //Color *= getAO(Pos-0*Normal, Normal);
	return  Color / 8;
	*/
}


[numthreads(4, 4, 4)]
void CS(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{

	uint3 index = GetVoxelLighting().GetParams().get_voxel_pos(dispatchID);

	float3 dims;
	output.GetDimensions(dims.x, dims.y, dims.z);

	float4 albedo = tex_albedo[index];

	if (albedo.w == 0)
	{
		output[index] = 0;
		return;
	}

	float3 normals = normalize(tex_normals[index].xyz * 2 - 1);

	float3 oneVoxelSize = voxel_info.GetSize() / (voxel_info.GetVoxel_tiles_count() * voxel_info.GetVoxels_per_tile());


	float m = max(max(abs(normals.x), abs(normals.y)), abs(normals.z));
#ifdef SECOND_BOUNCE
	float4 gi = getGI((index + normals / m) / dims, normals);
#else
	float4 gi = 0;
#endif


	float3 pos = index * voxel_size / dims + voxel_min + oneVoxelSize * normals ;

	float shadow = saturate(dot(normals, dir)) * get_shadow(pos);
	float3 lighting = 1 * albedo.xyz * gi.xyz + 1*albedo.xyz * shadow;

	output[index] = float4(lighting, 1);
}
