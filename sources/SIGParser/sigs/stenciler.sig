[Bind = DefaultLayout::Instance0]
struct Countour
{
	float4 color;
	Texture2D<float4> tex;
}

[Bind = DefaultLayout::Instance5]
struct DrawStencil
{
	StructuredBuffer<float4> vertices;
}

[Bind = DefaultLayout::Instance0]
struct PickerBuffer
{
	RWStructuredBuffer<uint> viewBuffer;
}

[Bind = DefaultLayout::Instance3]
struct Instance
{
	uint instanceId;
}


[Bind = DefaultLayout::Instance4]
struct Color
{
	float4 color;
}




GraphicsPSO DrawStencil
{
	root = DefaultLayout;

	[EntryPoint = VS]
	mesh = gbuffer/mesh_shader;

	[EntryPoint = AS]
	amplification = gbuffer/mesh_shader;

	[EntryPoint = PS]
	pixel = postprocess/stencil;

	ds = D32_FLOAT;
	cull = None;
	depth_func = GREATER;
}




GraphicsPSO DrawSelected
{
	root = DefaultLayout;

	[EntryPoint = VS]
	mesh = gbuffer/mesh_shader;

	[EntryPoint = AS]
	amplification = gbuffer/mesh_shader;

	[EntryPoint = PS_RESULT]
	pixel = postprocess/stencil;
	
	

	rtv = { R8_SNORM };

	enable_depth = false;
	cull = None;
	depth_func = ALWAYS;
}




GraphicsPSO DrawBox
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = postprocess/triangle_stencil;

	[EntryPoint = PS]
	pixel = postprocess/triangle_stencil;
	
	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}



GraphicsPSO DrawAxis
{
	root = DefaultLayout;

	[EntryPoint = VS]
	mesh = gbuffer/mesh_shader;

	[EntryPoint = AS]
	amplification = gbuffer/mesh_shader;


	[EntryPoint = PS_COLOR]
	pixel = postprocess/stencil;
	
	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
}


GraphicsPSO DrawRing
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = postprocess/ring;

	[EntryPoint = PS_COLOR]
	pixel = postprocess/ring;

	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
}


GraphicsPSO DrawRingPick
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = postprocess/ring;

	[EntryPoint = PS]
	pixel = postprocess/ring;

	ds = D32_FLOAT;
	cull = None;
	depth_func = GREATER;
}


GraphicsPSO StencilerLast
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = postprocess/contour;

	[EntryPoint = PS]
	pixel = postprocess/contour;
	
	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}

# Mirrors stencil_renderer::selected.empty() -- written once per frame by
# stencil_renderer::update_frame() (StencilRenderer.cpp), which also does the
# camera/gizmo work that used to live in stencil_renderer_before's setup().
struct StencilState
{
	bool has_selection = false;
}

[Required]
[RunAlways]
PassNode stencil_renderer_before
{
	# 1x1: this pass renders the picking gizmos into a throwaway depth
	# buffer -- only the stencil/color outputs (stencil_renderer_after)
	# matter, depth here exists purely to satisfy the graphics pipeline.
	[Always = DepthStencil] [Size = 1] [Format = R32_TYPELESS] Texture depth_tex;

	[Always = UnorderedAccess] [Size = 1] StructuredBuffer<UINT> id_buffer;
	[Always = UnorderedAccess] [Size = 1] StructuredBuffer<UINT> axis_id_buffer;
}

[SetupCondition = `builder.graph->get_context<Table::StencilState>().has_selection`]
PassNode stencil_renderer_after
{
	[Always = RenderTarget] Texture ResultTexture;
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R8_SNORM] Texture Stencil_color_tex;
}