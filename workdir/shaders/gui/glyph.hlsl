#include "../autogen/GlyphRender.h"
#include "display_output.hlsl"

struct glyph_output
{
	float4 pos : SV_POSITION;
	float4 color : TEXCOORD0;
	float2 tc : TEXCOORD1;
	nointerpolation uint texture_index : TEXCOORD2;
	nointerpolation uint is_color : TEXCOORD3;
	nointerpolation uint is_pattern : TEXCOORD4;
	nointerpolation float4 tile : TEXCOORD5;
};

static const StructuredBuffer<GlyphQuad> quads = GetGlyphRender().GetQuads();

#ifdef BUILD_FUNC_VS
glyph_output VS(uint index : SV_VERTEXID)
{
	// Two triangles per quad, expanded from the quad's corners.
	static const uint2 corners[6] = { uint2(0, 0), uint2(1, 0), uint2(0, 1), uint2(0, 1), uint2(1, 0), uint2(1, 1) };

	GlyphQuad q = quads[index / 6];
	uint2 c = corners[index % 6];

	float4 p = q.GetPos();
	float4 uv = q.GetUv();

	glyph_output o;
	o.pos = float4(c.x ? p.z : p.x, c.y ? p.w : p.y, 0.999, 1);
	o.tc = float2(c.x ? uv.z : uv.x, c.y ? uv.w : uv.y);
	o.color = q.GetColor();
	o.texture_index = q.GetAtlas();
	o.is_color = q.GetIs_color();
	o.is_pattern = q.GetIs_pattern();
	o.tile = q.GetTile();
	return o;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(glyph_output i) : SV_TARGET0
{
	// Bilinear: glyphs drawn at a non-integer size are scaled from a bitmap
	// rasterized at the rounded-up size. Pixel-snapped 1:1 quads sample texel
	// centers, where bilinear is exact.
	// A pattern's uv counts tiles along the quad; wrap it into the tile's atlas
	// rect (Skribidi insets the rect a texel, so bilinear stays inside it).
	float2 tc = i.is_pattern ? i.tile.xy + frac(i.tc) * (i.tile.zw - i.tile.xy) : i.tc;
	float4 s = GetGlyphRender().GetTextures(i.texture_index).Sample(linearClampSampler, tc);

	float4 result;
	if (i.is_color)
	{
		// Atlas color glyphs are premultiplied; the UI blend state is straight alpha.
		result.rgb = s.a > 0 ? s.rgb / s.a : 0;
		result.a = s.a * i.color.a;
	}
	else
	{
		result.rgb = i.color.rgb;
		result.a = s.r * i.color.a;
	}

	return ui_output(result);
}
#endif
