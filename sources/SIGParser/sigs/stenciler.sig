[Bind = DefaultLayout::Instance0]
struct Countour
{
	float4 color;
	Texture2D<float4> tex;
}

[Bind = DefaultLayout::Instance0]
struct DrawStencil
{
	StructuredBuffer<float4> vertices;
}

[Bind = DefaultLayout::Instance0]
struct PickerBuffer
{
	RWStructuredBuffer<uint> viewBuffer;
}

[Bind = DefaultLayout::Instance2]
struct Instance
{
	uint instanceId;
}


[Bind = DefaultLayout::Instance0]
struct Color
{
	float4 color;
}




GraphicsPSO DrawStencil
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = triangle;

	[EntryPoint = PS]
	pixel = stencil;

	ds = DXGI_FORMAT_D32_FLOAT;
	cull = NONE;
	depth_func = LESS;
}




GraphicsPSO DrawSelected
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = triangle;

	[EntryPoint = PS_RESULT]
	pixel = stencil;
	
	

	rtv = { DXGI_FORMAT_R8_SNORM };

	enable_depth = false;
	cull = NONE;
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
	cull = NONE;

	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	blend = { Additive };
}



GraphicsPSO DrawAxis
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = triangle;

	[EntryPoint = PS_COLOR]
	pixel = stencil;
	
	enable_depth = false;
	cull = NONE;

	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT };
}


GraphicsPSO StencilerLast
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = contour;

	[EntryPoint = PS]
	pixel = contour;
	
	enable_depth = false;
	cull = NONE;

	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	blend = { Additive };
}

