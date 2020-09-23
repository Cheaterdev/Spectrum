#pragma once
#include "Glyph.h"
struct FontRenderingGlyphs_srv
{
	StructuredBuffer<Glyph> data;
};
struct FontRenderingGlyphs
{
	FontRenderingGlyphs_srv srv;
	StructuredBuffer<Glyph> GetData() { return srv.data; }

};
 const FontRenderingGlyphs CreateFontRenderingGlyphs(FontRenderingGlyphs_srv srv)
{
	const FontRenderingGlyphs result = {srv
	};
	return result;
}
