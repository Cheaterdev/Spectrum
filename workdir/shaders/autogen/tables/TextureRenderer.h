#pragma once
struct TextureRenderer_srv
{
	uint texture; // Texture2D<float4>
};
struct TextureRenderer
{
	TextureRenderer_srv srv;
	Texture2D<float4> GetTexture() { return ResourceDescriptorHeap[srv.texture]; }

};
 const TextureRenderer CreateTextureRenderer(TextureRenderer_srv srv)
{
	const TextureRenderer result = {srv
	};
	return result;
}
