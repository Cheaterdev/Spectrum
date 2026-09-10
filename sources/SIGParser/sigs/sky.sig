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
	vertex = sky/sky;

	[EntryPoint = PS]
	pixel = sky/sky;

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}

ComputePSO SkyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = sky/sky;
}


ComputePSO SkyCube
{
	root = DefaultLayout;

	[EntryPoint = CS_Cube]
	compute = sky/sky;
}

ComputePSO CubemapENV
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = sky/cubemap_down;
}

ComputePSO CubemapENVDiffuse
{
	root = DefaultLayout;

	[EntryPoint = CS_Diffuse]
	compute = sky/cubemap_down;
}



[Compute]
PassNode Sky
{
	[Always = Read] Texture GBuffer_Depth;
	[Always = UnorderedAccess] Texture ResultTexture;
}


[Compute]
PassNode CubeSky
{
	# Not [Always]: setup() calls data.sky_cubemap.changed() right after
	# create()'ing it, and downstream passes (CubeMapDownsample,
	# CubeMapEnviromentProcessor) read is_changed() from their OWN setup() --
	# same phase, same frame. create_always()/need_always() only run after
	# setup_func returns, so calling .changed() from render() (tried once,
	# reverted) is too late: every setup() across the whole graph already
	# ran by the time any pass's render() executes, so downstream setups
	# would see is_changed() as permanently false and never re-filter the
	# cubemap. This has to stay a manual create() + changed() pair, together,
	# inside setup_func.
	[Write] TextureCube sky_cubemap;
}


[Static]
[Compute]
# TriState: need_always() must run every frame regardless of whether the sky
# actually changed (SetupResult::IgnoreRender on unchanged frames still runs
# it, unlike a plain `false` -> Disabled) -- the mip regeneration itself
# (render()) only needs to run on changed frames.
[TriState]
PassNode CubeMapDownsample
{
	[Always = UnorderedAccess] TextureCube sky_cubemap;
	TextureCube sky_cubemap_filtered;
	TextureCube sky_cubemap_filtered_diffuse;
}

[Static]
[Compute]
# TriState: sky_cubemap_filtered/diffuse must be create()'d every frame
# regardless of whether sky_cubemap actually changed (SetupResult::IgnoreRender
# on unchanged frames still runs create_always()), while render() -- the
# actual filter dispatch -- only needs to run on changed frames.
[TriState]
PassNode CubeMapEnviromentProcessor
{
	[Always = Read] TextureCube sky_cubemap;
	[Always = UnorderedAccess | Static] [Size = 64] [Format = R11G11B10_FLOAT] [MipCount = 0] TextureCube sky_cubemap_filtered;
	[Always = UnorderedAccess | Static] [Size = 64] [Format = R11G11B10_FLOAT] [MipCount = 0] TextureCube sky_cubemap_filtered_diffuse;
}