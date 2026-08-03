[Bind = DefaultLayout::Instance0]
struct MipMapping
{
	uint SrcMipLevel;
    uint NumMipLevels;
    float2 TexelSize;
	
	
	# Split from OutMip[4] — fixed-size arrays in CBV structs accessed via
	# ResourceDescriptorHeap cause DXC to emit duplicate OpTypeArray IDs in SPIR-V.
	# Four individual fields have identical binary layout and avoid the issue.
	# See REFACTOR_TODO.md for root cause and the proper template fix.
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


# Hi-Z pyramid mip-chain step: reads one mip, writes the next (exactly half
# size per axis, standard mip halving -- unlike DownsampleDepth above, which
# is a one-shot 8x reduction from a full-res depth buffer down to a single
# coarse level). Min-reduced (reversed-Z conservative), same semantics as
# DownsampleDepth, just 2x2->1 instead of 8x8->1 per output texel.
[Bind = DefaultLayout::Instance0]
struct DownsampleDepthMip
{
	Texture2D<float> srcMip;
	RWTexture2D<float> dstMip;
}

ComputePSO DownsampleDepthMip
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = downsample_depth_mip;
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


	ds = D32_FLOAT;
	cull = None;
	depth_func = ALWAYS;
}


GraphicsPSO QualityColor
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = gbuffer_quality;

	[EntryPoint = PS]
	pixel = gbuffer_quality;

	rtv = { R8G8_UNORM };
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
	stencil_pass_op = Replace;

	ds = D24_UNORM_S8_UINT;


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
	stencil_pass_op = Replace;

	ds = D24_UNORM_S8_UINT;


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

	[type = Format]
	[PS]
	define Format = { ALL_RT_FORMATS };

	rtv = { Format };
}

