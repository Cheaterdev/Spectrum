
struct vertex_input
{
	float2 pos;
	float2 tc;
	float4 mulColor;
	float4 addColor;
}

[Bind = DefaultLayout::Instance0]
struct NinePatch
{
	StructuredBuffer<vertex_input> vb;
	Texture2D<float4> textures[];
}

[Bind = DefaultLayout::Instance0]
struct ColorRect
{
	float4 pos[2];
	float4 color[4];
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

	rtv = { B8G8R8A8_UNORM };
	blend = { AlphaBlend };

	cull = None;
}


GraphicsPSO SimpleRect
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/rect;

	[EntryPoint = PS_COLOR]
	pixel = gui/rect;

	rtv = { B8G8R8A8_UNORM };
	blend = { AlphaBlend };
	cull = None;
}

GraphicsPSO CanvasBack
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/ninepatch;

	[EntryPoint = PS]
	pixel = gui/canvas;

	enable_depth = false;
	cull = None;
	topology = TRIANGLE;

	rtv = { B8G8R8A8_UNORM };
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
	cull = None;
	topology = PATCH;

	rtv = { B8G8R8A8_UNORM };
	blend = { AlphaBlend };
}


[Bind = DefaultLayout::Instance0]
struct FrameGraph_Debug_Common
{
	RWTexture2D<float4> target;
	uint2 targetSize;
	float3 minColor;
	float3 maxColor;
	uint selectedMip;
	uint selectedArrayIndex;
}

[Bind = DefaultLayout::Instance1]
struct FrameGraph_Debug_Texture2D
{
	Texture2D<float4> source;
	uint2 sourceSize;
	float2 scale;
	float2 offset;
}

ComputePSO FrameGraph_Debug_Texture2D
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = framegraph/draw_texture_2d;
}

[Bind = DefaultLayout::Instance1]
struct FrameGraph_Debug_Texture2DArray
{
	Texture2DArray<float4> source;
	uint2 sourceSize;
	float2 scale;
	float2 offset;
}

ComputePSO FrameGraph_Debug_Texture2DArray
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = framegraph/draw_texture_2d_array;
}


[Bind = DefaultLayout::Instance1]
struct FrameGraph_Debug_Texture3D
{
	Texture3D<float4> source;
	uint3 sourceSize;

	Camera camera;
}

ComputePSO FrameGraph_Debug_Texture3D
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = framegraph/draw_texture_3d;
}


[Bind = DefaultLayout::Instance1]
struct FrameGraph_Debug_TextureCube
{
	TextureCube<float4> source;
	uint2 sourceSize;
}

ComputePSO FrameGraph_Debug_TextureCube
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = framegraph/draw_texture_cube;
}



ComputePSO FrameGraph_Debug_NotImplemented
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = framegraph/draw_not_implemented;
}


[Bind = DefaultLayout::Instance0]
struct StatGraph
{
	uint  Count;
	uint  Width;
	uint  Height;
	float Vmin;
	float Vmax;

	float4 LineColor;
	float4 FillTop;
	float4 FillBot;
	float4 BgTop;
	float4 BgBot;

	StructuredBuffer<float> Samples;
	RWTexture2D<float4>     Output;
}

ComputePSO StatGraph
{
	root = DefaultLayout;
	[EntryPoint = CS]
	compute = gui/stat_graph;
}

[Bind = DefaultLayout::Instance0]
struct StatGraphLine
{
	uint  Count;
	uint  Width;
	uint  Height;
	float Vmin;
	float Vmax;

	float4 LineColor;
	float4 FillColor;
	float4 BgTop;
	float4 BgBot;

	StructuredBuffer<float> Samples;
}

GraphicsPSO StatGraphLines
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gui/stat_graph_lines;

	[EntryPoint = GS]
	geometry = gui/stat_graph_lines;

	[EntryPoint = PS]
	pixel = gui/stat_graph_lines;

	rtv = { R8G8B8A8_UNORM };
	blend = { AlphaBlend };
	cull = None;
	topology = LINE;
	enable_depth = false;
}


[Static]
[Required]
PassNode UI_PreDraw
{
	[Write] StructuredBuffer<uint> UI_PreDraw_Sync;
}

[Static]
[Multiple = 16]
PassNode UI_Render
{
	Texture swapchain;
	StructuredBuffer<uint> UI_PreDraw_Sync;
}


Pipeline UIPipeline
{
	Profiler;
	UI_PreDraw;
	UI_Render;
}

