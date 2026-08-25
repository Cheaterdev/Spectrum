[Bind = DefaultLayout::Instance0]
struct SkyData
{

	float3 sunDir;
	Texture2D<float> depthBuffer;

	Texture2D<float4> transmittance;
	Texture3D<float4> inscatter;
	Texture2D<float4> irradiance;
	RWTexture2D<float4> result;
}


[Bind = DefaultLayout::Instance1]
struct SkyFace
{
	# All six cube faces as one array UAV, so the bake is a single
	# (w, h, 6) dispatch with the face taken from the dispatch Z.
	RWTexture2DArray<float4> faces;
}


[Bind = DefaultLayout::Instance1]
struct EnvFilter
{
	# .x = edge length of the source cubemap (drives the solid-angle -> source
	#      mip estimate), .y = number of output mips packed into the dispatch,
	# .z = edge length of output mip 0.
	uint4 size;

	# One array UAV per output mip. The specular pass walks all of them from a
	# single flattened dispatch; the diffuse pass writes [0] only.
	RWTexture2DArray<float4> targets[8];
}


[Bind = DefaultLayout::Instance2]
struct EnvSource
{
	TextureCube<float4> sourceTex;
}


GraphicsPSO Sky
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = sky;

	[EntryPoint = PS]
	pixel = sky;

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}

ComputePSO SkyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = sky;
}


ComputePSO SkyCube
{
	root = DefaultLayout;

	[EntryPoint = CS_Cube]
	compute = sky;
}

ComputePSO CubemapENV
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = cubemap_down;
}

ComputePSO CubemapENVDiffuse
{
	root = DefaultLayout;

	[EntryPoint = CS_Diffuse]
	compute = cubemap_down;
}



[Compute]
PassNode Sky
{
	Texture GBuffer_Depth;
	[Write] Texture ResultTexture;
}


[Compute]
PassNode CubeSky
{
	[Write] TextureCube sky_cubemap;
}


[Static]
[Compute]
PassNode CubeMapDownsample
{
	[Write] TextureCube sky_cubemap;
	TextureCube sky_cubemap_filtered;
	TextureCube sky_cubemap_filtered_diffuse;
}

[Static]
[Compute]
PassNode CubeMapEnviromentProcessor
{
	TextureCube sky_cubemap;
	[Write] TextureCube sky_cubemap_filtered;
	[Write] TextureCube sky_cubemap_filtered_diffuse;
}