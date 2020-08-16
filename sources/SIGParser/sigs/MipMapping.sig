[Bind = DefaultLayout::Instance0]
struct MipMapping
{
	uint SrcMipLevel;
    uint NumMipLevels;
    float2 TexelSize;
	
	
	RWTexture2D<float4> OutMip[4];

	Texture2D<float4> SrcMip;

}


[Bind = DefaultLayout::Instance0]
struct CopyTexture
{
	Texture2D<float4> srcTex;
}

[Bind = DefaultLayout::Instance0]
struct DownsampleDepth
{
	Texture2D<float> srcTex;
	RWTexture2D<float> targetTex;
}

