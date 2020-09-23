[Bind = DefaultLayout::Instance0]
struct SkyData
{

	float3 sunDir;
	Texture2D depthBuffer;
 
	Texture2D transmittance;
	Texture3D inscatter;
	Texture2D irradiance;
}


[Bind = DefaultLayout::Instance1]
struct SkyFace
{
	uint face;
}


[Bind = DefaultLayout::Instance1]
struct EnvFilter
{
	uint4 face;	
	float4 scaler;
	uint4 size;
}


[Bind = DefaultLayout::Instance2]
struct EnvSource
{
	TextureCube sourceTex;
}


GraphicsPSO Sky
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = sky;

	[EntryPoint = PS]
	pixel = sky;

	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	blend = { Additive };
}


GraphicsPSO SkyCube
{
	root = DefaultLayout;

	[EntryPoint = VS_Cube]
	vertex = sky;

	[EntryPoint = PS_Cube]
	pixel = sky;

	rtv = { DXGI_FORMAT_R11G11B10_FLOAT };
}

GraphicsPSO CubemapENV
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = cubemap_down;

	[EntryPoint = PS]
	pixel = cubemap_down;

	[rename = NumSamples, indirect]
	[PS]
	define Level = {1,8,32,64,128};


	rtv = { DXGI_FORMAT_R11G11B10_FLOAT };
}

GraphicsPSO CubemapENVDiffuse
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = cubemap_down;

	[EntryPoint = PS_Diffuse]
	pixel = cubemap_down;

	rtv = { DXGI_FORMAT_R11G11B10_FLOAT };
}
