#include "2D_screen_simple.h"
#include "Formats.hlsl"

			
#ifdef BUILD_FUNC_PS
#include "autogen/CopyTexture.h"
static const Texture2D<float4> full_tex = GetCopyTexture().GetSrcTex();

type PS(quad_output i):SV_TARGET0
{

	type4 color= full_tex.SampleLevel(linearClampSampler, i.tc,0);
color=pow(color,1.0/2.2);
color.a = 1;

  return color;
}

#endif