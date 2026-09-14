
# Mirrors GUI::UIContext::pre_draw_infos.size() -- set once per frame right
# after the UI tree is walked (GUI/Base.cpp), read by UI_PreDraw's own
# [RenderCondition]. Not a bool: kept as a count so a future consumer that
# cares how many widgets asked for a pre-draw doesn't need a second field.
struct UIState
{
	uint UI_Passes_needed = 0;
}

# UI_Render (below) is [Multiple=16]: draw_infos gets split into per_pass-
# sized chunks, one per instance, and only the first passes_needed instances
# actually have work. Computed once per frame (GUI/Base.cpp, same block as
# UIState above) instead of each of the 16 instances recomputing it and
# instead of the old ui_ctx.setup_counter++ (an ad hoc per-instance ordinal
# -- data.pass_index, already generated for every [Multiple] pass, is the
# same value without a hand-rolled counter).
struct UIRenderState
{
	uint passes_needed = 0;
	uint per_pass = 0;
}

struct vertex_input
{
	float2 pos;
	float2 tc;
	float4 mulColor;
	float4 addColor;
	# Linear -> gamma encode (pow(x, 1/2.2)) for this draw. Used by the main
	# viewport image, which is fed a linear HDR render result.
	float gammaEncode;
}

[Bind = DefaultLayout::Instance0]
struct NinePatch
{
	StructuredBuffer<vertex_input> vb;

	# [Auto]: a UI image whose texture was never assigned -- a preview panel
	# opened for a cell that has no thumbnail is the common one -- otherwise
	# writes descriptor index 0 into the array and the pixel shader samples
	# whatever sits at the start of the heap (GBV #940, once per such image
	# drawn, hundreds after clicking around the FrameGraph debugger).
	[Auto = Texture_Null]
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
# setup() must create() UI_PreDraw_Sync every frame so its ResourceChain
# resets even when there's nothing to pre-draw, but should still skip
# render() on those frames -- [RenderCondition] alone (no [SetupCondition])
# says exactly that: IgnoreRender, never Disabled, so create_always() still
# runs every frame regardless of UI_Passes_needed.
[RenderCondition = UIState::UI_Passes_needed > 0]
PassNode UI_PreDraw
{
	[Always = UnorderedAccess | Required] [Size = 1] StructuredBuffer<uint> UI_PreDraw_Sync;
}

[Static]
[Multiple = 16]
# data.pass_index is this instance's own ordinal, generated for every
# [Multiple] pass -- so the "is there work for this instance" test is a plain
# condition and needs no hand-written setup().
[SetupCondition = data.pass_index < UIRenderState::passes_needed]
# The debug-view source: its ResourceID is picked at runtime from
# DebugContext::mode (GUI/Base.cpp's create_graph), so no fixed field can name
# it. See [NeedDynamic]'s generated comment (need_always, pass/UI_Render.h) for
# why the expression has to be that lvalue specifically.
[NeedDynamic = `builder.graph->get_context<FrameGraph::DebugContext>().result_texture`]
[NeedDynamicFlags = Read]
PassNode UI_Render
{
	[Always = RenderTarget] Texture swapchain;
	# [Optional]: UI_PreDraw_Sync may not exist this frame (its own producer,
	# UI_PreDraw, can be disabled) -- guarded exactly like the hand-written
	# builder.exists() check it replaces.
	[Always = Read] [Optional = exists(UI_PreDraw_Sync)]
	StructuredBuffer<uint> UI_PreDraw_Sync;
}


[Multiple = 16]
[Required]
# One instance per asset-preview widget that asked for a GPU pass this frame --
# a preview window, or a material graph's embedded output preview, and several
# can be live at once (main.cpp's asset_preview_content).
#
# NOT [Static]: render() is a call into a specific widget instance, so the owner
# fills render_funcs[i] and registration itself is the per-instance gate -- the
# pipeline adds only the slots that are filled, which is exactly why the setup
# half can be an unconditional [RunAlways] rather than a [SetupCondition]
# counting live previews through a Table:: context.
[RunAlways]
# Declares no resources on purpose: the preview reads a standalone asset
# texture and writes its own, neither of them graph-tracked, so nothing links
# this pass into the graph -- hence [Required], or it would be culled for
# writing nothing.
PassNode AssetPreview
{
}


Pipeline UIPipeline
{
	Profiler;
	AssetPreview;
	UI_PreDraw;
	UI_Render;
}

