

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

	# GBuffer_HalfDepth/HalfNormals + TileClassifyHi/Low/Mask, generic infra
	# for any pass below wanting a "does this tile need full-res work"
	# signal -- see TileClassifyData's own comment (pssm.sig). Must precede
	# every consumer; not [Async] since it reads this frame's just-finished
	# GBuffer directly.
	GBufferDownsampler;
	[Async]NormalRoughnessRepack;
	[Async]ReflectionRTXHalf;
	[Async]ReflectionRTX;
	[Async]ShadowRTX;
	[Async]IndirectRTXHalf;
	[Async]IndirectRTX;
	# Voxel-cone-traced alternative sources for NRD_REBLUR_Execute below
	# (selected via g_indirect_source/g_reflection_source, see
	# [[project-nrd-integration]]) -- run after Mipmapping (VoxelLighted is
	# ready by here) and before NRD_GBufferPack, which now reads their raw
	# output too (packs every candidate unconditionally, see its own .sig
	# comment) -- NRD_GBufferPack must come after both its RTX producers
	# (IndirectRTX/ReflectionRTX, above) and its VCT producers (here), or
	# builder.need() on a not-yet-created resource null-derefs in
	# ResourceChain::active() (crashed here once already).
	[Async]VoxelScreen;
	[Async]ScreenReflection;
	[Async]NRD_GBufferPack;

											[Async]
											RTXShadow;
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
											[Async2]VSM_ScreenSpaceShadow;
											[Async2]VSM_ShadowResolve;
											[Async2]VSM_Combine;
											# Debug-only overlay, after VSM_ShadowResolve/VSM_Combine so it
											# paints on top of whichever one actually shaded the result -- see
											# its own PassNode comment in vsm.sig.
											[Async2]VSM_DebugClassifyOverlay;

		[Async]NRD_REBLUR_Execute;
		# FSR/DLSS-side equivalent of the indirect term RTXCombine computes
		# for DLSS-RR -- see nrd_sig_test.sig's own PassNode comment and
		# [[project-nrd-integration]].
		[Async]NRD_IndirectCombine;
		[Async]ReflCombine;
		[Async]RTXCombine;
		# RTX-reference shadow (see vsm.sig's own VSMSelectors::shadow_source
		# comment, [[project-nrd-integration]]) -- runs after VSM's own chain
		# above so its overwrite of ResultTexture's shadow term is the last
		# word whenever RTXReference is selected.
		[Async]NRD_SIGMA_Execute;
		[Async]NRD_ShadowCombine;

	# sky + post
		[Async]Sky;
	
	[Async]SMAA;
	[Async]FSR;
	UpscalingDLSS;
	UpscalingDLSSRR;

	stencil_renderer_after;

	#Debug stuff

	RTXColorPass;
	VoxelDebug;
}

