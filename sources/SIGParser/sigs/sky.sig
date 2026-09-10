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
	# create_always() runs every frame this pass is enabled, regardless of
	# whether the sun actually moved (SetupResult::IgnoreRender vs
	# NeedsRender) -- Static means the underlying allocation only happens
	# once. The .changed() mutator moved to render(), which only runs on
	# NeedsRender frames, after create_always() has already linked the
	# handle -- see Sky.cpp's CubeSky setup/render split.
	[Always = UnorderedAccess | Static] [Size = 256] [Format = R11G11B10_FLOAT] [MipCount = 0]
	TextureCube sky_cubemap;
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
# TriState: sky_cubemap_filtered/diffuse must be create()'d every frame
# regardless of whether sky_cubemap actually changed (SetupResult::IgnoreRender
# on unchanged frames still runs create_always()), while render() -- the
# actual filter dispatch -- only needs to run on changed frames.
[TriState]
PassNode CubeMapEnviromentProcessor
{
	# Not [Always]: setup()'s own return value depends on
	# data.sky_cubemap.is_changed(), which must be queried after this
	# field's own need() runs within the SAME setup() call -- see
	# project_sig_auto_need_and_caching's is_new()/is_changed() exclusion.
	TextureCube sky_cubemap;
	[Always = UnorderedAccess | Static] [Size = 64] [Format = R11G11B10_FLOAT] [MipCount = 0] TextureCube sky_cubemap_filtered;
	[Always = UnorderedAccess | Static] [Size = 64] [Format = R11G11B10_FLOAT] [MipCount = 0] TextureCube sky_cubemap_filtered_diffuse;
}