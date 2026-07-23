

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
	#[Async] 
	BlueNoise;

	# voxel pre (generate_pre)
	Voxelize;

	PSSM_Global;
	PSSM_Cascade;

	# shadow (generate_global)
	Scene;
	stencil_renderer_before;

	# sky setup (sky.generate)
	CubeSky;
	CubeMapDownsample;
	CubeMapEnviromentProcessor;

	# voxel lighting (generate_light)
	

	#[Async] 
	Lighting;
	#[Async] 
	Mipmapping;


	RTXShadow;

	# main gbuffer + rtx

	# result target
	ResultCreation;

	# shadow composition (pssm.generate)
	PSSM_GenerateMask;
	PSSM_Combine;

	# voxel screen (voxel_gi.generate)
	VoxelScreen;
	VoxelCombine;
	ScreenReflection;
	ReflectionDenoiser_Reproject;
	ReflCombine;
	VoxelDebug;
	RTXColorPass;

	# sky + post
	Sky;
	
	stencil_renderer_after;
	SMAA;
	FSR;

	# frame end
	CopyPrev;
}

