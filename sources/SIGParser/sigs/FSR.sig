
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
	compute = FSR;
}




ComputePSO RCAS
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = FSR;


	[rename = RCAS]
	[CS]
	define cas;
}
