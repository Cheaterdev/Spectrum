
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

struct Glyph
{
	float2 pos;
	uint index;
	float4 color;
}

[Bind = DefaultLayout::Instance2]
struct FontRenderingGlyphs
{
	StructuredBuffer<Glyph> data;
}

GraphicsPSO FontRender
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = font/vsSimple;

	[EntryPoint = PS]
	pixel = font/psSimple;

	[EntryPoint = GS]
	geometry = font/gsSimple;

	topology = POINT;

	[type = Format]
	[PS]
	define Format = { ALL_RT_FORMATS };

	rtv = { Format };

	enable_depth = false;
}