
#include "autogen/FrameClassification.h"


static const Texture2D<float> frames_tex = GetFrameClassification().GetFrames();


static const AppendStructuredBuffer<uint2> hi = GetFrameClassification().GetHi();
static const AppendStructuredBuffer<uint2> low = GetFrameClassification().GetLow();


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
    float frames_min = 1;
	for(uint x = 0;x<32;x++)
        for (uint y = 0; y < 32; y++)
        {
            float frames = frames_tex[32*dispatchID.xy + uint2(x,y)];

            frames_min = min(frames_min, frames);
        }

	

uint index = groupThreadID.x+groupThreadID.y*8;
//if (index == 0)

	if(frames_min<0.9)
{
	//if(groupID.x==2)
   hi.Append(dispatchID);
    }
    else
    {
        low.Append(dispatchID);
    }
	//if(index == 0)
  //      hi.Append(dispatchID.xy);
}
