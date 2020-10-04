#define SIZE 16
#define FIX 8
#include "2D_screen_simple.h"


#include "Common.hlsl"
#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelDebug.h"

static const Camera camera = GetFrameInfo().GetCamera();
static const Camera prevCamera = GetFrameInfo().GetPrevCamera();
static const VoxelInfo voxel_info = GetVoxelInfo();
static const GBuffer gbuffer = GetVoxelDebug().GetGbuffer();
static const Texture3D<float4> volume = GetVoxelDebug().GetVolume();


static const float3 voxel_min = voxel_info.GetMin().xyz;
static const float3 voxel_size = voxel_info.GetSize().xyz;

//#define voxel_min float3(-150,-150,-150)
//#define voxel_size float3(300,300,300)

float4 get_voxel(float3 pos, float level)
{
	float4 color =  volume.SampleLevel(linearSampler, pos,level);
//color.rgb *= 1 + level / 2;
//color.w *= 2;
if (color.w > 0) color /= color.w;


	color*= all(pos > 0) * all(pos < 1);
	return color;// *all(pos > 0)* all(pos < 1);
}
float get_alpha(float3 pos, float level)
{
return volume.SampleLevel(linearSampler,pos,level).a;
}



float4 trace_screen(float3 origin, float3 dir, float level)
{
	origin = ((origin - (voxel_min)) / voxel_size);
	dir = normalize(dir);
	float3 samplePos = 0;
	float4 accum = 0;
	// the starting sample diameter
	float minDiameter = 1.0 / 512;// *(1 + 2 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;
	// push out the starting point to avoid self-intersection
	float startDist = 0;// get_start_dist(origin, dir, minDiameter, angle);

	float maxDist = 1;
	float dist = startDist;

	while (dist <= maxDist && accum.w < 1 )
	{
		//	float sampleDiameter = minDiameter + level;//max(minDiameter, angle * dist);
		float sampleDiameter = minDiameter + level * dist;

		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);

		samplePos = origin + dir * dist;
		float4 sampleValue = get_voxel(samplePos, sampleLOD);

		/*if (sampleValue.w > 0)
		{
			if (level == 0)
				return sampleValue / sampleValue.w;
			else
				level =saturate(level-0.1);
		}
		else
		{
			level = saturate(level + 0.1);
			dist += minDiameter*sampleDiameter;// pow(2, level);
		}*/
		float sampleWeight = saturate(1 - accum.w);
		accum += sampleValue * sampleWeight;
		dist += sampleDiameter;// pow(2, level);
	}


	return accum;
}

float4 Debug(quad_output i) :SV_Target0
{
	

	float3 pos = depth_to_wpos(0.5, i.tc, camera.GetInvViewProj());
	float3 v = normalize(pos - camera.GetPosition());

	float4 result = trace_screen(camera.GetPosition(), v, 0.005);
	result *= result.w;
	result.w = 1;
	return result;

	}
