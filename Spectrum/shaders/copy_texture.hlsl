#include "2D_screen_simple.h"
#include "autogen/CopyTexture.h"

static const Texture2D<float4> full_tex = GetCopyTexture().GetSrcTex();

float4 PS(quad_output i):SV_TARGET0
{
  return float4(pow(full_tex.SampleLevel(linearClampSampler, i.tc,0).rgb,1.0/2.2),1);
}
