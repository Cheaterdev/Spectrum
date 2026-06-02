

[Bind = DefaultLayout::Instance0]
struct Test
{
	float data[16];
	Texture2D<float4> tex[];
	StructuredBuffer<MeshInstanceInfo> instances[];
}


Pipeline UIPipeline
{
	# Minimal pipeline for UI-only rendering (Vulkan bootstrap, no 3D passes).
	# Profiler keeps the swapchain alive as a required RT; UI_Render draws all
	# registered GUI elements.  No ResultTexture, no GBuffer — swapchain is the
	# direct render target.
	Profiler;
	UI_Render;
}

Pipeline MainPipeline
{
	# scene prep
	PreScene;
	BlueNoise;

	# voxel pre (generate_pre)
	Voxelize;

	# shadow (generate_global)
	Scene;
	PSSM_Global;
	PSSM_Cascade;

	# sky setup (sky.generate)
	CubeSky;
	CubeMapDownsample;
	CubeMapEnviromentProcessor;

	# voxel lighting (generate_light)
	
	RTXPass;
	Lighting;
	Mipmapping;

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

	# sky + post
	Sky;
	stencil_renderer_before;
	stencil_renderer_after;
	SMAA;
	FSR;

	# frame end
	CopyPrev;
}

