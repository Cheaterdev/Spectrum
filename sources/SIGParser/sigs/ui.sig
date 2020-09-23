
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
	float4 pos[2];
	float4 color;
}



[Bind = DefaultLayout::Instance1]
struct FlowGraph
{
	float4 size;
	float4 offset_size;
	float2 inv_pixel;
}

GraphicsPSO NinePatch
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/ninepatch;

	[EntryPoint = PS]
	pixel = gui/ninepatch;

	rtv = { DXGI_FORMAT_R8G8B8A8_UNORM };
	blend = { AlphaBlend };
}


GraphicsPSO SimpleRect
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/rect;

	[EntryPoint = PS_COLOR]
	pixel = gui/rect;

	rtv = { DXGI_FORMAT_R8G8B8A8_UNORM };
	blend = { AlphaBlend };
	cull = NONE;
}

GraphicsPSO CanvasBack
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/ninepatch;

	[EntryPoint = PS]
	pixel = gui/canvas;

	enable_depth = false;
	cull = NONE;
	topology = TRIANGLE;

	rtv = { DXGI_FORMAT_R8G8B8A8_UNORM };
	blend = { AlphaBlend };
}

struct VSLine
{
	float2 pos;
	float4 color;
}

[Bind = DefaultLayout::Instance0]
struct LineRender
{
	StructuredBuffer<VSLine> vb;
}


GraphicsPSO CanvasLines
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/flow_line;

	[EntryPoint = PS]
	pixel = gui/flow_line;

	[EntryPoint = GS]
	geometry = gui/flow_line;

	[EntryPoint = DS]
	domain = gui/flow_line;

	[EntryPoint = HS]
	hull = gui/flow_line;

	enable_depth = false;
	cull = NONE;
	topology = PATCH;

	rtv = { DXGI_FORMAT_R8G8B8A8_UNORM };
	blend = { AlphaBlend };
}