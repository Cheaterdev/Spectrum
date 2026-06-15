
[Bind = DefaultLayout::Instance0]
struct BRDF
{
	RWTexture3D<float4> output;	
}


[ExcludeVulkan] ComputePSO BRDF
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = BRDF;
}