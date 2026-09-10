
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


[Static]
[Compute]
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