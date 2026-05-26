
[Bind = DefaultLayout::Instance0]
struct TextureRenderer
{
	Texture2D<float4> texture;
}

PassNode AssetGBuffer
{
	GBuffer gbuffer;
}

PassNode AssetMip
{
	Texture ResultTexture;
}