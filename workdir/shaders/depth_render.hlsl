#include "2D_screen_simple.h"

#include "autogen/CopyTexture.h"

static const Texture2D<float4> full_tex = GetCopyTexture().GetSrcTex();

#ifdef BUILD_FUNC_PS
float PS(quad_output i) : SV_Depth
{

    return full_tex.SampleLevel(pointClampSampler, i.tc,0);
}
#endif