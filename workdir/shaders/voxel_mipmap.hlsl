//
// Copyright (c) Microsoft. All rights reserved.
#include "autogen/VoxelMipMap.h"
#include "autogen/VoxelInfo.h"


static const  RWTexture3D<float4> OutMip1 = GetVoxelMipMap().GetOutMips(0);
static const RWTexture3D<float4> OutMip2 = GetVoxelMipMap().GetOutMips(1);
static const RWTexture3D<float4> OutMip3 = GetVoxelMipMap().GetOutMips(2);
static const Texture3D<float4> SrcMip = GetVoxelMipMap().GetSrcMip();

static const VoxelInfo voxel_info = GetVoxelInfo();


void calc(inout float4 color)
{
	color.rgb/= color.w + 0.5;
	color.w = (color.w/4);

	//todo: investigate negative values
	color = max(0, color);
}

void add_color(inout float4 result, float4 c)
{
	result += float4(c.rgb* c.w,c.w);
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

	calc(c);
		
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
