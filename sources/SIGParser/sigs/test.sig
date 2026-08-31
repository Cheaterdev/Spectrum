

enum TestEnumMode
{
	None;
	FirstMode;
	SecondMode = 5;
}

[Bind = DefaultLayout::Instance0]
struct Test
{
	float data[16];
	TestEnumMode mode = None;
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
											# Phase 5.18 Part A: must run before VSM_BlockerClassify/VSM_BlockerSearch
											# now (both read VSM_PageHiZ) -- same [Async2] queue so that
											# ordering is ordinary same-queue in-order execution, not a new
											# cross-queue fence. See VSM_HiZRebuild's own comment in
											# vsm.sig. Moved here from its previous spot near the end of
											# the pipeline.
											[Async2]VSM_HiZRebuild;
											# Phase 5.18 Part A follow-up (take 4): three stages, in order --
											# classify builds the tile lists, search runs indirectly over
											# just the ambiguous ones, resolve issues the three per-tile PSOs
											# (full-lit/full-shadow/shadow-blur) that write the final shadow
											# value. See vsm.sig's own PassNode comments.
											[Async2]VSM_BlockerClassify;
											[Async2]VSM_BlockerSearch;
											[Async2]VSM_ShadowResolve;
											[Async2]VSM_Combine;
											# Debug-only overlay, after VSM_Combine so it paints on top of
											# the already-shaded result -- see its own PassNode comment in
											# vsm.sig.
											[Async2]VSM_DebugClassifyOverlay;
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

	stencil_renderer_after;

	#Debug stuff

	RTXColorPass;
	VoxelDebug;
}

