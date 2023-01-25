

[Bind = DefaultLayout::Instance2]
struct GBufferDownsample
{
	Texture2D<float4> normals;
	Texture2D<float> depth;
}



[Bind = DefaultLayout::Instance2]
struct GBufferQuality
{
	Texture2D<float4> ref;
}

[Bind = DefaultLayout::Instance0]
struct PSSMConstants
{
	int level;
	float time;
}

[Bind = DefaultLayout::Instance1]
struct PSSMData
{	
	Texture2DArray<float> light_buffer;
	StructuredBuffer<Camera> light_cameras;
}

[Bind = DefaultLayout::Instance1]
struct PSSMDataGlobal
{	
	Texture2D<float> light_buffer;
	StructuredBuffer<Camera> light_camera;
}


[Bind = DefaultLayout::Instance2]
struct PSSMLighting
{
	GBuffer gbuffer;
	Texture2D<float> light_mask;
}


GraphicsPSO PSSMMask
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = PSSM;

	[EntryPoint = PS]
	pixel = PSSM;

	rtv = { R8_UNORM };
}

GraphicsPSO PSSMApply
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = PSSM;

	[EntryPoint = PS_RESULT]
	pixel = PSSM;

	rtv = { R16G16B16A16_FLOAT };
}


[RenderTarget]
struct GBufferDownsampleRT
{
	RenderTarget<float> depth;
	RenderTarget<float4> color;
}


GraphicsPSO GBufferDownsample
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = Downsample;

	[EntryPoint = PS]
	pixel = Downsample;

	rtv = { R32_FLOAT, R8G8B8A8_UNORM };
}
