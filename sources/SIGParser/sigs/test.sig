

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
	CubeSky;
	CubeMapDownsample;
	CubeMapEnviromentProcessor;

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

	ScreenReflection;
	ReflectionDenoiser_Reproject;

											[Async]
											RTXShadow;
											[Async]PSSM_Combine;
											VSM_DepthAnalysis;
											VSM_Combine;
											# voxel screen (voxel_gi.generate)
											[Async]
											VoxelScreen;
											[Async]
											VoxelCombine;
	[Async]VSM_HiZRebuild;
	ReflCombine;

	# sky + post
	Sky;
	
	SMAA;
	FSR;
	UpscalingDLSS;
	
	stencil_renderer_after;

	#Debug stuff

	RTXColorPass;
	VoxelDebug;
}

