
struct FSRConstants
{
	uint4 Const0;
	uint4 Const1;
	uint4 Const2;
	uint4 Const3;
	uint4 Sample;
}

[Bind = DefaultLayout::Instance0]
struct FSR
{
	FSRConstants constants;
	Texture2D<float4> source;
	RWTexture2D<float4> target;
}




ComputePSO FSR
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = postprocess/fsr;
}




ComputePSO RCAS
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = postprocess/fsr;


	[rename = RCAS]
	[CS]
	define cas;
}


# Runs when FSR is the selected upscaler -- which, since upscaler_type can
# only ever hold an available type (see g_upscaler_type's invariant,
# UpscalingDLSS.ixx), already covers "DLSS/DLSS-RR was asked for but this
# hardware can't run it": set_upscaler_type() clamped that to FSR at the
# point of selection, so there is nothing left to re-derive here.
[Static]
[Compute]
[SetupCondition = UpscalerSelectors::upscaling_enabled && UpscalerSelectors::upscaler_type == UpscalerType::FSR]
PassNode FSR
{
	# MipCount=0: full auto mip chain, matching the original manual recreate()
	# call's omitted 4th Desc field (value-initializes to 0, not the more
	# common single-mip 1 -- see FrameGraph.cpp's mip_count==0 auto-chain path).
	[Always = RenderTarget] [Recreate = ResultTextureNew] [RecreateFlags = UnorderedAccess]
	[Size = ViewportContext::upscale_size] [Format = R16G16B16A16_FLOAT] [MipCount = 0]
	Texture ResultTexture;

	[Always = UnorderedAccess] [Size = ViewportContext::upscale_size] [Format = R16G16B16A16_FLOAT] Texture FSRTemp;
}