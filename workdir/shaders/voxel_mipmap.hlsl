//
// Copyright (c) Microsoft. All rights reserved.
#include "autogen/VoxelMipMap.h"
#include "autogen/VoxelInfo.h"


static const  RWTexture3D<float4> OutMip1 = GetVoxelMipMap().GetOutMips(0);
static const RWTexture3D<float4> OutMip2 = GetVoxelMipMap().GetOutMips(1);
static const RWTexture3D<float4> OutMip3 = GetVoxelMipMap().GetOutMips(2);
static const Texture3D<float4> SrcMip = GetVoxelMipMap().GetSrcMip();

static const VoxelInfo voxel_info = GetVoxelInfo();


// For the first stage the /8 average is done by the trilinear fetch itself;
// the groupshared stages below still sum 8 values and need the divide.
void calc_avg(inout float4 color)
{
	color.w=saturate(color.w*1.3f);
	//todo: investigate negative values
	color = max(0, color);
}

void calc(inout float4 color)
{
    color /= 8;
	calc_avg(color);
}

void add_color(inout float4 result, float4 c)
{
	result += float4(c.rgb,c.w);
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
 uint3 index = GetVoxelMipMap().GetParams().get_voxel_pos(dispatchID);

	float3 dims;
	SrcMip.GetDimensions(dims.x, dims.y, dims.z);

	// One trilinear fetch at the 2x2x2 block corner IS the 8-texel average —
	// the filtering hardware does the 8 loads + divide. Unmapped source tiles
	// read as 0, so loaded output tiles over unloaded input regions come out
	// zero without a separate clear pass.
	float4 c = SrcMip.SampleLevel(linearClampSampler, (2 * index + 1) / dims, 0);

	calc_avg(c);

	OutMip1[index] = c;

	#if COUNT >= 2
	data[local_index] = c;
	#endif
	
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

		calc(c);
		
		OutMip2[index/2] = c;
		
		data[local_index] = c;	
	}
	#endif
	
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

		calc(c);
		OutMip3[index/4] = c;
	}
	#endif
}
