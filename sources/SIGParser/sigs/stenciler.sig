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
	mesh = mesh_shader;

	[EntryPoint = AS]
	amplification = mesh_shader;

	[EntryPoint = PS]
	pixel = stencil;

	ds = D32_FLOAT;
	cull = None;
	depth_func = GREATER;
}




GraphicsPSO DrawSelected
{
	root = DefaultLayout;

	[EntryPoint = VS]
	mesh = mesh_shader;

	[EntryPoint = AS]
	amplification = mesh_shader;

	[EntryPoint = PS_RESULT]
	pixel = stencil;
	
	

	rtv = { R8_SNORM };

	enable_depth = false;
	cull = None;
	depth_func = ALWAYS;
}




GraphicsPSO DrawBox
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = triangle_stencil;

	[EntryPoint = PS]
	pixel = triangle_stencil;
	
	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}



GraphicsPSO DrawAxis
{
	root = DefaultLayout;

	[EntryPoint = VS]
	mesh = mesh_shader;

	[EntryPoint = AS]
	amplification = mesh_shader;


	[EntryPoint = PS_COLOR]
	pixel = stencil;
	
	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
}


GraphicsPSO DrawRing
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = ring;

	[EntryPoint = PS_COLOR]
	pixel = ring;

	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
}


GraphicsPSO DrawRingPick
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = ring;

	[EntryPoint = PS]
	pixel = ring;

	ds = D32_FLOAT;
	cull = None;
	depth_func = GREATER;
}


GraphicsPSO StencilerLast
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = contour;

	[EntryPoint = PS]
	pixel = contour;
	
	enable_depth = false;
	cull = None;

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}

[Required]
PassNode stencil_renderer_before
{
	Texture depth_tex;

	StructuredBuffer<UINT> id_buffer;
	StructuredBuffer<UINT> axis_id_buffer;
}

PassNode stencil_renderer_after
{
	Texture ResultTexture;
	Texture Stencil_color_tex;
}