#define SIZE 16
#define FIX 8
#include "2D_screen_simple.h"

#include "autogen/FrameInfo.h"
#include "autogen/DenoiserDownsample.h"

static const Camera camera = GetFrameInfo().GetCamera();

static const DenoiserDownsample denoiser_downsample = GetDenoiserDownsample();

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


static const Texture2D<float4> color_tex = denoiser_downsample.GetColor();


static const int2 delta[4] =
{
    int2(0, 1),
    int2(1, 1),
    int2(1, 0),
    int2(0, 0)
};

float4  PS(quad_output input) : SV_TARGET0
{

    float2 dims;

color_tex.GetDimensions(dims.x, dims.y);


float4 best_color = 0;

[unroll] for (int j = 0; j < 4; j++)
{
    float4 c = color_tex.SampleLevel(pointClampSampler, input.tc, 0, delta[j]);

   // if (c.w < best_color.w)
    {
        best_color += c;
    }	
}
  
  
return best_color/4;// float4(color_tex.SampleLevel(pointClampSampler, input.tc + float2(delta[offset]) / dims, 0).xyzw);
}

