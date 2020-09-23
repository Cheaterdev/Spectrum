#pragma once
struct Glyph_cb
{
	float2 pos;
	uint index;
	float4 color;
};
struct Glyph
{
	Glyph_cb cb;
	float2 GetPos() { return cb.pos; }
	uint GetIndex() { return cb.index; }
	float4 GetColor() { return cb.color; }

};
 const Glyph CreateGlyph(Glyph_cb cb)
{
	const Glyph result = {cb
	};
	return result;
}
