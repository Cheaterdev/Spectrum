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
	// The glyph's own atlas rect: with a shadow the quad is larger than the glyph, and only
	// inside this rect is there coverage to sample.
	nointerpolation float4 uv_rect : TEXCOORD6;
	// Where the shadow's coverage is sampled relative to this pixel's own: the offset in uv,
	// and how far its blur taps reach in uv.
	nointerpolation float2 shadow_shift : TEXCOORD7;
	nointerpolation float2 shadow_blur_uv : TEXCOORD8;
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

	// A pattern (a wavy underline tile) has no shadow of its own.
	const bool shadowed = GetGlyphRender().GetShadow_color().a > 0 && !q.GetIs_pattern();
	const float2 shift = shadowed ? GetGlyphRender().GetShadow_offset() : float2(0, 0);
	const float2 blur = shadowed ? GetGlyphRender().GetShadow_blur() : float2(0, 0);

	// The quad grows to hold the glyph and the glyph moved by the shadow offset, plus the
	// blur's reach all round; uvs follow linearly (ndc y runs up, so p.w < p.y).
	float2 size = p.zw - p.xy;
	float2 uv_per_ndc = (uv.zw - uv.xy) / size;
	float4 grown = float4(min(p.x, p.x + shift.x) - blur.x, max(p.y, p.y + shift.y) + blur.y,
	                      max(p.z, p.z + shift.x) + blur.x, min(p.w, p.w + shift.y) - blur.y);
	float2 corner = float2(c.x ? grown.z : grown.x, c.y ? grown.w : grown.y);

	glyph_output o;
	o.pos = float4(corner, 0.999, 1);
	o.tc = uv.xy + (corner - p.xy) * uv_per_ndc;
	o.color = q.GetColor();
	o.texture_index = q.GetAtlas();
	o.is_color = q.GetIs_color();
	o.is_pattern = q.GetIs_pattern();
	o.tile = q.GetTile();
	o.uv_rect = uv;
	o.shadow_shift = shift * uv_per_ndc;
	o.shadow_blur_uv = blur * abs(uv_per_ndc);
	return o;
}
#endif

#ifdef BUILD_FUNC_PS
// The glyph's coverage at tc, zero outside its atlas rect (the neighbours in the atlas are not
// part of this glyph).
float coverage(glyph_output i, float2 tc)
{
	float2 lo = min(i.uv_rect.xy, i.uv_rect.zw);
	float2 hi = max(i.uv_rect.xy, i.uv_rect.zw);

	if (any(tc < lo) || any(tc > hi))
		return 0;

	float4 s = GetGlyphRender().GetTextures(i.texture_index).Sample(linearClampSampler, tc);
	return i.is_color ? s.a : s.r;
}

float4 PS(glyph_output i) : SV_TARGET0
{
	// Bilinear: glyphs drawn at a non-integer size are scaled from a bitmap
	// rasterized at the rounded-up size. Pixel-snapped 1:1 quads sample texel
	// centers, where bilinear is exact.
	// A pattern's uv counts tiles along the quad; wrap it into the tile's atlas
	// rect (Skribidi insets the rect a texel, so bilinear stays inside it).
	float2 tc = i.is_pattern ? i.tile.xy + frac(i.tc) * (i.tile.zw - i.tile.xy) : i.tc;

	const float4 shadow_color = GetGlyphRender().GetShadow_color();
	const bool shadowed = shadow_color.a > 0 && !i.is_pattern;

	float4 result;

	if (shadowed)
	{
		// The glyph over its own shadow, in one pass: the shadow's coverage is the glyph's
		// sampled where this pixel minus the offset lies.
		float glyph = coverage(i, tc);

		// Five taps: the middle one and one a blur radius out on each axis.
		float2 at = tc - i.shadow_shift;
		float2 b = i.shadow_blur_uv;
		float soft = coverage(i, at) * 0.36
		           + (coverage(i, at + float2(b.x, 0)) + coverage(i, at - float2(b.x, 0))
		            + coverage(i, at + float2(0, b.y)) + coverage(i, at - float2(0, b.y))) * 0.16;
		float shade = soft * shadow_color.a * i.color.a;

		float3 glyph_rgb = i.color.rgb;
		float glyph_alpha = glyph * i.color.a;

		if (i.is_color)
		{
			float4 s = GetGlyphRender().GetTextures(i.texture_index).Sample(linearClampSampler, tc);
			glyph_rgb = s.a > 0 ? s.rgb / s.a : 0;
		}

		// "Over": the glyph on top, the shadow behind; straight alpha out.
		float out_alpha = glyph_alpha + shade * (1 - glyph_alpha);
		result.a = out_alpha;
		result.rgb = out_alpha > 0 ? (glyph_rgb * glyph_alpha + shadow_color.rgb * shade * (1 - glyph_alpha)) / out_alpha : 0;
	}
	else
	{
		float4 s = GetGlyphRender().GetTextures(i.texture_index).Sample(linearClampSampler, tc);

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
	}

	return GetGlyphRender().GetRaw_output() ? result : ui_output(result);
}
#endif
