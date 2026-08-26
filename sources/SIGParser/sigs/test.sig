

[Bind = DefaultLayout::Instance0]
struct Test
{
	float data[16];
	Texture2D<float4> tex[];
	StructuredBuffer<MeshInstanceInfo> instances[];
}

Pipeline MainPipeline
{
	# scene prep
	PreScene;
											[Async]
											BlueNoise;

	# voxel pre (generate_pre)
	Voxelize;

	PSSM_Global;
	PSSM_Cascade;
	VSM_GatherDispatch;
	VSM_RenderPages;


	# shadow (generate_global)
	Scene;


	# sky setup (sky.generate)
	[Async2]CubeSky;
	[Async2]CubeMapDownsample;
	[Async2]CubeMapEnviromentProcessor;

											# voxel lighting (generate_light)
											[Async]
											Lighting;
											[Async]
											Mipmapping;
	stencil_renderer_before;


	# result target
	ResultCreation;

	# shadow composition (pssm.generate)
	PSSM_GenerateMask;

	[Async]ScreenReflection;
	[Async]ReflectionDenoiser_Reproject;

											[Async]
											RTXShadow;
											[Async]PSSM_Combine;
											[Async2]VSM_DepthAnalysis;
											[Async2]VSM_BlockerSearch;
											[Async2]VSM_Combine;
											# voxel screen (voxel_gi.generate)
											[Async2]
											VoxelScreen;
											[Async2]
											VoxelCombine;
	
		[Async]ReflCombine;

	# sky + post
		[Async]Sky;
	
	[Async]SMAA;
	[Async]FSR;
	UpscalingDLSS;


	[Async]VSM_HiZRebuild;
	stencil_renderer_after;

	#Debug stuff

	RTXColorPass;
	VoxelDebug;
}

