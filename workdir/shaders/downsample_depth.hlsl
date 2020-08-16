
#include "autogen/DownsampleDepth.h"


static const Texture2D<float> depth_tex = GetDownsampleDepth().GetSrcTex();
static const RWTexture2D<float> result_tex = GetDownsampleDepth().GetTargetTex();


static const uint TotalNumThreads =8*8;
groupshared float data[TotalNumThreads];

[numthreads(8, 8, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{

uint index = groupThreadID.x+groupThreadID.y*8;
uint2 tc = (groupThreadID.xy+groupID.xy*8);

data[index] =  depth_tex[tc];

GroupMemoryBarrierWithGroupSync();

	[unroll(TotalNumThreads)]
	for(uint s = TotalNumThreads / 2; s > 0; s >>= 1)
	{
		if(index < s)
			data[index] = max(data[index],data[index + s]);

		GroupMemoryBarrierWithGroupSync();
	}

	// Have the first thread write out to the output texture
	if(index == 0)
		result_tex[groupID.xy] = data[0];
}
