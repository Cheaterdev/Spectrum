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

RWTexture3D<float4> OutMip1 : register(u0);
RWTexture3D<float4> OutMip2 : register(u1);
//#if COUNT == 3
RWTexture3D<float4> OutMip3 : register(u2); 
//#endif
Texture3D<float4> SrcMip : register(t0);
SamplerState BilinearClamp : register(s0);
StructuredBuffer<int3> voxel_visibility: register(t1);


#define DIV

void calc(inout float4 color)
{

//	color.rgb /= color.w;
	//color /= color.w;


float v = color.w;
#define ADD 0.4

//color.w = pow(color.w,0.5);
//color.w /= color.w + ADD;
//color.w=saturate(color.w*(1.2));
//color= saturate(color*(2));
//

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
	result += float4(c.rgb*c.w,c.w);
//	result += c;
}
cbuffer consts:register(b0)
{
	uint4 voxels_per_tile;
};

uint3 get_index(uint3 groupThreadID, uint3 groupID)
{

	uint3 tile_index = voxel_visibility[groupID.x / voxels_per_tile.w];
	int tile_local_index = groupID.x%voxels_per_tile.w;
	uint3 index = tile_index*voxels_per_tile.xyz + groupThreadID + 4 * int3(tile_local_index % 8, (tile_local_index / 8) % 4, tile_local_index / (4 * 8));//groupID*4+groupThreadID;
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

   float3 dims;
   SrcMip.GetDimensions(dims.x, dims.y, dims.z);
   
	uint local_index = groupThreadID.x+groupThreadID.y*4+groupThreadID.z*16;
  uint3 index = get_index(groupThreadID, groupID);// groupID * 4 + groupThreadID;
	float4 color = SrcMip.SampleLevel(BilinearClamp,(2*index+1)/dims,0);
	
	calc(color);
		
	OutMip1[index] = color;
	data[local_index] = color;
	
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
		//color.rgb /= 8;// color.w;
		c.rgb /= c.w+0.01;

		c.w = saturate(c.w / 4);
		//c.w /= 8;
	calc(c);
		
		OutMip2[index/2] = c;
		
		data[local_index] = c;	
		
	
	}
	#endif
	
	
		#if COUNT == 2
		OutMip1[index] = 0;
	 
		OutMip2[index/2] =0;
		
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
		c.rgb /= c.w + 0.01;
		c.w = saturate(c.w / 4);
		//c.w /= 8;
	calc(c);
		OutMip3[index/4] = c;
	}
	#endif
}
