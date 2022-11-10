#include "../autogen/FontRendering.h"
#include "../autogen/FontRenderingConstants.h"


static const Texture2D<float> tex0 = GetFontRendering().GetTex0();
static const float4x4  TransformMatrix = GetFontRenderingConstants().GetTransformMatrix();

struct PSIn
{
	float4 Position : SV_Position;
	float4 GlyphColor : COLOR;
	float2 TexCoord : TC;
};

float4 PS(PSIn Input) : SV_Target
{
   float a = tex0.Sample(pointClampSampler, Input.TexCoord);
//   return float4(Input.TexCoord, 0, 1);
 //    return 1;

//  if (a == 0.0f)
//       discard;


return float4(Input.GlyphColor.rgb, a * Input.GlyphColor.a);
}