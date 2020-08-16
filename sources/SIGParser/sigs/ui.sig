
struct vertex_input
{
	float2 pos;
	float2 tc;
}

[Bind = DefaultLayout::Instance0]
struct NinePatch
{
	StructuredBuffer<vertex_input> vb;
	Texture2D textures[];
}

[Bind = DefaultLayout::Instance0]
struct ColorRect
{
	float4 color;
}



[Bind = DefaultLayout::Instance1]
struct FlowGraph
{
	float4 size;
	float4 offset_size;
	float2 inv_pixel;
}
