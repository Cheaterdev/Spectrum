export module FrameGraphAutogen:Passes.;

import FrameGraph;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;

export
{
	enum class PassID
	{
		AssetGBuffer,
		AssetMip,
		BlueNoise,
		ReflectionDenoiser_Reproject,
		ShadowDenoiser_Prepare,
		ShadowDenoiser_TileClassification,
		ShadowDenoiser_Filter,
		FSR,
		ResultCreation,
		PSSM_Cascade,
		PSSM_GenerateMask,
		PSSM_Combine,
		PSSM_Global,
		RTXPass,
		PreScene,
		Profiler,
		CopyPrev,
		Scene,
		Sky,
		CubeSky,
		CubeMapDownsample,
		CubeMapEnviromentProcessor,
		SMAA,
		stencil_renderer_before,
		stencil_renderer_after,
		UI_Render,
		GBufferDownsampler,
		VoxelDebug,
		VoxelScreen,
		VoxelCombine,
		ScreenReflection,
		ReflCombine,
		Voxelize,
		Lighting,
		Mipmapping
	};

	struct Library
	{
	
			Library::AssetGBuffer AssetGBuffer;
		Library::AssetMip AssetMip;
		Library::BlueNoise BlueNoise;
		Library::ReflectionDenoiser_Reproject ReflectionDenoiser_Reproject;
		Library::ShadowDenoiser_Prepare ShadowDenoiser_Prepare;
		Library::ShadowDenoiser_TileClassification ShadowDenoiser_TileClassification;
		Library::ShadowDenoiser_Filter ShadowDenoiser_Filter;
		Library::FSR FSR;
		Library::ResultCreation ResultCreation;
		Library::PSSM_Cascade PSSM_Cascade;
		Library::PSSM_GenerateMask PSSM_GenerateMask;
		Library::PSSM_Combine PSSM_Combine;
		Library::PSSM_Global PSSM_Global;
		Library::RTXPass RTXPass;
		Library::PreScene PreScene;
		Library::Profiler Profiler;
		Library::CopyPrev CopyPrev;
		Library::Scene Scene;
		Library::Sky Sky;
		Library::CubeSky CubeSky;
		Library::CubeMapDownsample CubeMapDownsample;
		Library::CubeMapEnviromentProcessor CubeMapEnviromentProcessor;
		Library::SMAA SMAA;
		Library::stencil_renderer_before stencil_renderer_before;
		Library::stencil_renderer_after stencil_renderer_after;
		Library::UI_Render UI_Render;
		Library::GBufferDownsampler GBufferDownsampler;
		Library::VoxelDebug VoxelDebug;
		Library::VoxelScreen VoxelScreen;
		Library::VoxelCombine VoxelCombine;
		Library::ScreenReflection ScreenReflection;
		Library::ReflCombine ReflCombine;
		Library::Voxelize Voxelize;
		Library::Lighting Lighting;
		Library::Mipmapping Mipmapping;
	
	};


	static Library Passes;
}