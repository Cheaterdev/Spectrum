#pragma once
struct FontRendering_srv
{
	Texture2D<float> tex0;
	Buffer<float4> positions;
};
struct FontRendering
{
	FontRendering_srv srv;
	Texture2D<float> GetTex0() { return srv.tex0; }
	Buffer<float4> GetPositions() { return srv.positions; }
};
 const FontRendering CreateFontRendering(FontRendering_srv srv)
{
	const FontRendering result = {srv
	};
	return result;
}
