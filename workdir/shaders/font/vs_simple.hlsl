#include "../autogen/FontRenderingGlyphs.h"

	struct VSOut {
		float3 Position : POSITION;
		float4 GlyphColor : COLOR;
		
	};
	
	VSOut VS(uint index: SV_VERTEXID) {
		VSOut Output;
		
		Glyph glyph = GetFontRenderingGlyphs().GetData()[index];

		Output.Position.xy = glyph.GetPos();
		Output.Position.z = glyph.GetIndex();
		Output.GlyphColor =  glyph.GetColor();
		
		return Output;
	}
	