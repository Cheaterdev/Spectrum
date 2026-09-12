
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


# Runs when explicitly selected, or as the automatic fallback when the
# selected type (DLSS/DLSS-RR) isn't actually available on this hardware --
# FSR is the only one of the three with no hardware gate. Equivalent to the
# old upscaler_is_available(g_upscaler_type) switch (UpscalingDLSS.ixx),
# inlined here since [SetupCondition] can only read Table:: contexts:
# DLSS/DLSSRR each check their own RenderDeviceCapabilities flag, FSR itself
# has none.
[Static]
[Compute]
[SetupCondition = `builder.graph->get_context<Table::UpscalerSelectors>().upscaling_enabled && !((builder.graph->get_context<Table::UpscalerSelectors>().upscaler_type == UpscalerType::DLSS && builder.graph->get_context<Table::RenderDeviceCapabilities>().dlss_available) || (builder.graph->get_context<Table::UpscalerSelectors>().upscaler_type == UpscalerType::DLSSRR && builder.graph->get_context<Table::RenderDeviceCapabilities>().dlssrr_available))`]
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