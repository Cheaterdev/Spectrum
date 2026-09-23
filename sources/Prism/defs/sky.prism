# "Has the sun moved since the last bake?" -- written once per frame by
# CubeSky's [PreSetup] hook (Sky.cpp), read by CubeSky's own and
# CubeMapDownsample's/CubeMapEnviromentProcessor's [RenderCondition]. Replaces
# the old resource-level is_changed()/changed() pair, which needed create()
# and the mutator to run in the same function.
#
# Its own Table:: context rather than a field on SkyInfo because a generated
# [RenderCondition] body lives in autogen/pass_defaults.cpp, which only ever
# sees SIG-declared Table:: contexts, not Graphics-layer structs.
#
# prev_sun_dir is the comparison state, and it lives HERE rather than as a
# SkyRender member on purpose: pre_setup() is static, and a Table:: context is
# per-Graph, so the main and asset pipelines -- each with their own SkyRender
# instance -- keep separate sun histories exactly as they did before.
struct SkyState
{
	float3 prev_sun_dir;
	bool sky_changed = false;
}

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
	vertex = "sky/sky.hlsl";

	[EntryPoint = PS]
	pixel = "sky/sky.hlsl";

	rtv = { R16G16B16A16_FLOAT };
	blend = { Additive };
}

ComputePSO SkyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = "sky/sky.hlsl";
}


ComputePSO SkyCube
{
	root = DefaultLayout;

	[EntryPoint = CS_Cube]
	compute = "sky/sky.hlsl";
}

ComputePSO CubemapENV
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = "sky/cubemap_down.hlsl";
}

ComputePSO CubemapENVDiffuse
{
	root = DefaultLayout;

	[EntryPoint = CS_Diffuse]
	compute = "sky/cubemap_down.hlsl";
}



[Compute]
[RunAlways]
PassNode Sky
{
	[Always = Read] Texture GBuffer_Depth;
	[Always = UnorderedAccess] Texture ResultTexture;
}


[Compute]
# [PreSetup] does the sun-direction diff into Table::SkyState (Sky.cpp) --
# a real side effect, and one CubeMapDownsample/CubeMapEnviromentProcessor's
# own conditions depend on, so it has to run before ANY pass's setup rather
# than inside this one's.
[PreSetup]
[RenderCondition = SkyState::sky_changed]
PassNode CubeSky
{
	# create_always() runs every frame this pass is enabled (SetupResult::
	# IgnoreRender on an unchanged frame still runs it -- Static means the
	# underlying allocation only happens once). Safe now that "did the sky
	# change" is tracked via Table::SkyState::sky_changed (see its own
	# comment above) rather than the old resource-level is_changed()/
	# changed() pair, which needed create() and the mutator to run in the
	# same function.
	[Always = UnorderedAccess | Static] [Size = 256] [Format = R11G11B10_FLOAT] [MipCount = 0]
	TextureCube sky_cubemap;
}


[Static]
[Compute]
# need_always() must run every frame regardless of whether the sky actually
# changed ([RenderCondition] never returns Disabled, only IgnoreRender/
# NeedsRender -- unlike a plain `false` -> Disabled) -- the mip regeneration
# itself (render()) only needs to run on changed frames.
[RenderCondition = SkyState::sky_changed]
PassNode CubeMapDownsample
{
	[Always = UnorderedAccess] TextureCube sky_cubemap;
	TextureCube sky_cubemap_filtered;
	TextureCube sky_cubemap_filtered_diffuse;
}

[Static]
[Compute]
# sky_cubemap_filtered/diffuse must be create()'d every frame regardless of
# whether sky_cubemap actually changed ([RenderCondition] never returns
# Disabled, so create_always() still runs), while render() -- the actual
# filter dispatch -- only needs to run on changed frames.
[RenderCondition = SkyState::sky_changed]
PassNode CubeMapEnviromentProcessor
{
	[Always = Read] TextureCube sky_cubemap;
	[Always = UnorderedAccess | Static] [Size = 64] [Format = R11G11B10_FLOAT] [MipCount = 0] TextureCube sky_cubemap_filtered;
	[Always = UnorderedAccess | Static] [Size = 64] [Format = R11G11B10_FLOAT] [MipCount = 0] TextureCube sky_cubemap_filtered_diffuse;
}