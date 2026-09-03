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

[Required]
PassNode stencil_renderer_before
{
	[Write] Texture depth_tex;

	[Write] StructuredBuffer<UINT> id_buffer;
	[Write] StructuredBuffer<UINT> axis_id_buffer;
}

PassNode stencil_renderer_after
{
	[Write] Texture ResultTexture;
	[Write] Texture Stencil_color_tex;
}