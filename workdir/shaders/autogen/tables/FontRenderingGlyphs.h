#pragma once
#include "Glyph.h"
struct FontRenderingGlyphs_srv
{
	uint data; // StructuredBuffer<Glyph>
};
struct FontRenderingGlyphs
{
	FontRenderingGlyphs_srv srv;
	StructuredBuffer<Glyph> GetData() { return ResourceDescriptorHeap[srv.data]; }

};
 const FontRenderingGlyphs CreateFontRenderingGlyphs(FontRenderingGlyphs_srv srv)
{
	const FontRenderingGlyphs result = {srv
	};
	return result;
}
