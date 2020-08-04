#define SIZE 16
#define FIX 8
#include "2D_screen_simple.h"

#include "autogen/FrameInfo.h"
#include "autogen/GBufferDownsample.h"

static const Camera camera = GetFrameInfo().GetCamera();
static const GBufferDownsample gbuffer = GetGBufferDownsample();

float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
    return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc, d, 1));
    return P.xyz / P.w;
}


static const Texture2D<float> depth_tex = gbuffer.GetDepth();
static const Texture2D<float4> normal_tex = gbuffer.GetNormals();


struct PS_RESULT
{
float depth: SV_TARGET0;
float4 normal : SV_TARGET1;
};


static const int2 delta[4] =
{
    int2(0, 1),
    int2(1, 1),
    int2(1, 0),
    int2(0, 0)
};

PS_RESULT PS(quad_output input)
{
	PS_RESULT result;
	float2 low_dimensions;
	depth_tex.GetDimensions(low_dimensions.x, low_dimensions.y);
  // i.tc-= 0.5/ low_dimensions;
	float depths[4] = (float[4]) depth_tex.Gather(PixelSampler, input.tc );
	float4 normals[4];

    [unroll] for (int j = 0; j < 4; j++)
	{
	//	depths[j]= depth_tex.Sample(PixelSampler, i.tc, delta[j]).x;
		float4 n = normal_tex.Sample(PixelSampler, input.tc, delta[j]);
		normals[j] = float4(normalize(n.xyz * 2 - 1), n.w);
}
    float mind = depths[0];
    float4 minN = normals[0];
    float maxd = depths[0];
    float4 maxN = normals[0];
    float sumd = depths[0];
    float3 sumn = normals[0];

    [unroll] for (int i = 1; i < 4; i++)
    {
        if (depths[i] < mind)
        {
            mind = depths[i];
            minN = normals[i];
        }

        if (depths[i] > maxd)
        {
            maxd = depths[i];
            maxN = normals[i];
        }

        sumd += depths[i];
        sumn += normals[i];
    }

    //result.depth = float4((sumd - mind - maxd) / 2,0,0,1);
    //  result.normal = float4(((sumn - minN - maxN) /2).xyz * 0.5 + 0.5, 1);
    result.depth = float4(mind, 0, 0, 1);
    result.normal = float4(minN.xyz * 0.5 + 0.5, minN.w);
  // result.depth = float4(maxd, 0, 0, 1);
 // result.normal = float4(maxN.xyz * 0.5 + 0.5, maxN.w);
    //result.depth = float4(depths[0].xxx, 1) ;
    // result.normal  = float4(normals[0], 1) ;
    return result;
}

PS_RESULT PS_Copy(quad_output i)
{
	PS_RESULT result;
	float2 low_dimensions;
	depth_tex.GetDimensions(low_dimensions.x, low_dimensions.y);
	
	result.depth = depth_tex.Sample(PixelSampler, i.tc);
	result.normal = 0;
	// result.depth = float4(maxd, 0, 0, 1);
   // result.normal = float4(maxN.xyz * 0.5 + 0.5, maxN.w);
	  //result.depth = float4(depths[0].xxx, 1) ;
	  // result.normal  = float4(normals[0], 1) ;
	return result;
}
