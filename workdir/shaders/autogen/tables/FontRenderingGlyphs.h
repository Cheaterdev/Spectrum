#pragma once
#include "sig_hlsl.hlsl"
#include "Glyph.h"
struct FontRenderingGlyphs
{
	uint data; // StructuredBuffer<Glyph>
	StructuredBuffer<Glyph> GetData() { return ResourceDescriptorHeap[data]; }
};