
[Bind = DefaultLayout::Instance0]
struct FontRendering
{
	Texture2D<float> tex0;
	Buffer<float4> positions;
}

[Bind = DefaultLayout::Instance1]
struct FontRenderingConstants
{
	float4x4 TransformMatrix;
	float4 ClipRect;
}
