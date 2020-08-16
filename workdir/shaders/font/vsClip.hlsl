#include "../autogen/FontRendering.h"
#include "../autogen/FontRenderingConstants.h"


static const Buffer<float4> tex0 = GetFontRendering().GetPositions();
static const float4x4  TransformMatrix = GetFontRenderingConstants().GetTransformMatrix();

	struct VSIn {
		float4 Position : POSITION;
		float4 GlyphColor : GLYPHCOLOR;
	};
	
	struct VSOut {
		float4 Position : SV_Position;
		float4 GlyphColor : COLOR;
		float2 TexCoord : TEXCOORD;
		float4 ClipDistance : CLIPDISTANCE;
	};
	
	VSOut VS(VSIn Input) {
		VSOut Output;
		
		Output.Position = mul(TransformMatrix, float4(Input.Position.xy, 0.0f, 1.0f));
		Output.GlyphColor = Input.GlyphColor;
		Output.TexCoord = Input.Position.zw;
		Output.ClipDistance = ClipRect + float4(Input.Position.xy, -Input.Position.xy);
		
		return Output;
	}
	;