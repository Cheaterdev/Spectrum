[Bind = DefaultLayout::Instance0]
struct MipMapping
{
	uint SrcMipLevel;
    uint NumMipLevels;
    float2 TexelSize;
	
	
	RWTexture2D<float4> OutMip[4];

	Texture2D<float4> SrcMip;

}


[Bind = DefaultLayout::Instance0]
struct CopyTexture
{
	Texture2D<float4> srcTex;
}

[Bind = DefaultLayout::Instance0]
struct DownsampleDepth
{
	Texture2D<float> srcTex;
	RWTexture2D<float> targetTex;
}




ComputePSO DownsampleDepth
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = downsample_depth;
}



ComputePSO MipMapping
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = GenerateMips;

	[rename = NON_POWER_OF_TWO]
	[CS]
	define NonPowerOfTwo = {0,1,2,3};

	[rename = CONVERT_TO_SRGB]
	[CS]
	define Gamma;
}




GraphicsPSO RenderToDS
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = depth_render;

	[EntryPoint = PS]
	pixel = depth_render;


	ds = DXGI_FORMAT_D32_FLOAT;
	cull = NONE;
	depth_func = ALWAYS;
}


GraphicsPSO QualityColor
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gbuffer_quality;

	[EntryPoint = PS]
	pixel = gbuffer_quality;

	rtv = { DXGI_FORMAT_R8G8_UNORM };
	depth_write = false;

}


GraphicsPSO QualityToStencil
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gbuffer_quality;

	[EntryPoint = PS_STENCIL]
	pixel = gbuffer_quality;

	enable_stencil = true;
	enable_depth = false;
	stencil_func = ALWAYS;
	stencil_pass_op = REPLACE;

	ds = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;


	#make as define block
	stencil_read_mask = 1;
	stencil_write_mask = 1;

}

#remove it
GraphicsPSO QualityToStencilREfl
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gbuffer_quality;

	[EntryPoint = PS_STENCIL]
	pixel = gbuffer_quality;

	enable_stencil = true;
	enable_depth = false;
	stencil_func = ALWAYS;
	stencil_pass_op = REPLACE;

	ds = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;


	# make as define block
	stencil_read_mask = 2;
	stencil_write_mask = 2;

}



GraphicsPSO CopyTexture
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = copy_texture;

	[EntryPoint = PS]
	pixel = copy_texture;

	enable_depth = false;

	[type = DXGI_FORMAT]
	define Format = { ALL_RT_FORMATS };

	rtv = { Format };
}
