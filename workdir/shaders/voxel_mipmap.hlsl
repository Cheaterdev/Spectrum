//
// Copyright (c) Microsoft. All rights reserved.
#include "autogen/VoxelMipMap.h"
#include "autogen/VoxelInfo.h"


static const  RWTexture3D<float4> OutMip1 = GetVoxelMipMap().GetOutMips(0);
static const RWTexture3D<float4> OutMip2 = GetVoxelMipMap().GetOutMips(1);
static const RWTexture3D<float4> OutMip3 = GetVoxelMipMap().GetOutMips(2);
static const Texture3D<float4> SrcMip = GetVoxelMipMap().GetSrcMip();
//static const SamplerState BilinearClamp : register(s0);
static const StructuredBuffer<int3> voxel_visibility = GetVoxelMipMap().GetVisibility();

static const VoxelInfo voxel_info = GetVoxelInfo();
static const uint group_count = GetVoxelMipMap().GetGroupCount();
#define DIV

void calc(inout float4 color)
{

//	color.rgb /= color.w+0.01;
//	color.w /= color.w+0.01;


float v = color.w;
#define ADD 0.2
//color = float4(0, 1, 0, 1);
//color.w = pow(color.w,0.5);
//color.w /= color.w + ADD;
//color.w=saturate(color.w*(1.2));
//color= saturate(color*(2));
//

}


void calc2(inout float4 color)
{
	//if (color.w > 0)
	//color.xyz /= color.w+0.01;
	color.rgb/= color.w + 0.5;
	

	color.w /= 4;

		//color.xyz /= color.w + 0.1;
	//	color/= 8;
//	color = saturate(color/ (color.w+0.1));
//	else
	//	color = 0;

	calc(color);
}


/*
cbuffer CB : register(b0)
{
    uint SrcMipLevel;	// Texture level of source mip
    uint NumMipLevels;	// Number of OutMips to write: [1, 4]
    float2 TexelSize;	// 1.0 / OutMip1.Dimensions
}
*/
void add_color(inout float4 result, float4 c)
{
	result += float4(c.rgb* c.w,c.w);
//	result += c;
}



uint3 get_index(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / GetVoxelInfo().GetVoxels_per_tile().x;
	uint3 tile_pos = voxel_visibility[tile_index] * GetVoxelInfo().GetVoxels_per_tile().xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index * GetVoxelInfo().GetVoxels_per_tile().x, 0, 0);
	uint3 index = tile_pos + tile_local_pos;
	return index;
}
groupshared float4 data[4*4*4];
[numthreads(4, 4, 4)]
void CS(    
	uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex)
{

 //  float3 dims;
  // SrcMip.GetDimensions(dims.x, dims.y, dims.z);
   
 uint local_index = groupThreadID.x+groupThreadID.y*4+groupThreadID.z*16;
  uint3 index = get_index(dispatchID);// groupID * 4 + groupThreadID;



//	float4 color = SrcMip.SampleLevel(linearSampler,(2*index+1)/dims,0);
	float4 c = 0;

	int4 tc = int4(2 * index , 0);

	add_color(c, SrcMip.Load(tc, int3(0, 0, 0)));
	add_color(c, SrcMip.Load(tc, int3(0, 0, 1)));
	add_color(c, SrcMip.Load(tc, int3(0, 1, 0)));
	add_color(c, SrcMip.Load(tc, int3(0, 1, 1)));

	add_color(c, SrcMip.Load(tc, int3(1, 0, 0)));
	add_color(c, SrcMip.Load(tc, int3(1, 0, 1)));
	add_color(c, SrcMip.Load(tc, int3(1, 1, 0)));
	add_color(c, SrcMip.Load(tc, int3(1, 1, 1)));
	/*add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	add_color(c, SrcMip.Load(2 * index + uint3(0, 0, 0));
	*/

	calc2(c);
		
	OutMip1[index] = c;
	data[local_index] = c;
	
	#if COUNT >=2
	GroupMemoryBarrierWithGroupSync();

		
	//mip2
	if(groupThreadID.x%2==0&&groupThreadID.y%2==0&&groupThreadID.z%2==0)
	{
		float4 c = 0;


		add_color(c, data[local_index+0*1+0*4+0*16]);
		add_color(c, data[local_index+0*1+0*4+1*16]);
		add_color(c, data[local_index+0*1+1*4+0*16]);
		add_color(c, data[local_index+0*1+1*4+1*16]);
		add_color(c, data[local_index+1*1+0*4+0*16]);
		add_color(c, data[local_index+1*1+0*4+1*16]);
		add_color(c, data[local_index+1*1+1*4+0*16]);
		add_color(c, data[local_index+1*1+1*4+1*16]);

	calc2(c);
		
		OutMip2[index/2] = c;
		
		data[local_index] = c;	
		
	
	}
	#endif
	
	/*
		#if COUNT == 2
		OutMip1[index] = 0;
	 
		OutMip2[index/2] =0;
		
		#endif*/
	#if COUNT == 3
	GroupMemoryBarrierWithGroupSync();
	
	//mip2
	if(local_index==0)
	{
		float4 c = 0;
		add_color(c, data[local_index+0*2+0*8+0*32]);
		add_color(c, data[local_index+0*2+0*8+1*32]);
		add_color(c, data[local_index+0*2+1*8+0*32]);
		add_color(c, data[local_index+0*2+1*8+1*32]);
		add_color(c, data[local_index+1*2+0*8+0*32]);
		add_color(c, data[local_index+1*2+0*8+1*32]);
		add_color(c, data[local_index+1*2+1*8+0*32]);
		add_color(c, data[local_index+1*2+1*8+1*32]);

	calc2(c);
		OutMip3[index/4] = c;
	}
	#endif
}
