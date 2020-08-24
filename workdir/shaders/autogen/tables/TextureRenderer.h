#pragma once
struct TextureRenderer_srv
{
	Texture2D<float4> texture;
};
struct TextureRenderer
{
	TextureRenderer_srv srv;
	Texture2D<float4> GetTexture() { return srv.texture; }

};
 const TextureRenderer CreateTextureRenderer(TextureRenderer_srv srv)
{
	const TextureRenderer result = {srv
	};
	return result;
}
