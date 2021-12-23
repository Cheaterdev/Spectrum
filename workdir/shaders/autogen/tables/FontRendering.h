#pragma once
struct FontRendering_srv
{
	uint tex0; // Texture2D<float>
	uint positions; // Buffer<float4>
};
struct FontRendering
{
	FontRendering_srv srv;
	Texture2D<float> GetTex0() { return ResourceDescriptorHeap[srv.tex0]; }
	Buffer<float4> GetPositions() { return ResourceDescriptorHeap[srv.positions]; }

};
 const FontRendering CreateFontRendering(FontRendering_srv srv)
{
	const FontRendering result = {srv
	};
	return result;
}
