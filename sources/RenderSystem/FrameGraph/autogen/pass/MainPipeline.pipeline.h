
#include "PreScene.h"
#include "BlueNoise.h"
#include "Voxelize.h"
#include "PSSM_Global.h"
#include "PSSM_Cascade.h"
#include "CubeSky.h"
#include "CubeMapDownsample.h"
#include "CubeMapEnviromentProcessor.h"
#include "Lighting.h"
#include "Mipmapping.h"
#include "Scene.h"
#include "RTXPass.h"
#include "ResultCreation.h"
#include "PSSM_GenerateMask.h"
#include "PSSM_Combine.h"
#include "VoxelScreen.h"
#include "VoxelCombine.h"
#include "ScreenReflection.h"
#include "ReflectionDenoiser_Reproject.h"
#include "ReflCombine.h"
#include "VoxelDebug.h"
#include "Sky.h"
#include "stencil_renderer_before.h"
#include "stencil_renderer_after.h"
#include "SMAA.h"
#include "FSR.h"
#include "CopyPrev.h"
#include "Profiler.h"
#include "../pass_defaults.h"

using namespace FrameGraph;

namespace Pipelines
{

class MainPipeline
{
public:

	Passes::PreScene preScene;
	Passes::BlueNoise blueNoise;
	Passes::Voxelize voxelize;
	Passes::PSSM_Global pSSM_Global;
	Passes::PSSM_Cascade pSSM_Cascade;
	Passes::CubeSky cubeSky;
	Passes::Lighting lighting;
	Passes::Mipmapping mipmapping;
	Passes::Scene scene;
	Passes::RTXPass rTXPass;
	Passes::PSSM_GenerateMask pSSM_GenerateMask;
	Passes::PSSM_Combine pSSM_Combine;
	Passes::VoxelScreen voxelScreen;
	Passes::VoxelCombine voxelCombine;
	Passes::ScreenReflection screenReflection;
	Passes::ReflectionDenoiser_Reproject reflectionDenoiser_Reproject;
	Passes::ReflCombine reflCombine;
	Passes::VoxelDebug voxelDebug;
	Passes::Sky sky;
	Passes::stencil_renderer_before stencil_renderer_before;
	Passes::stencil_renderer_after stencil_renderer_after;
	Passes::SMAA sMAA;

	void add_passes(FrameGraph::Graph& graph)
	{

		if (preScene.setup_func)
			graph.add_library_pass<Passes::PreScene>(preScene.setup_func, preScene.render_func, preScene.flags);
		if (blueNoise.setup_func)
			graph.add_library_pass<Passes::BlueNoise>(blueNoise.setup_func, blueNoise.render_func, blueNoise.flags);
		if (voxelize.setup_func)
			graph.add_library_pass<Passes::Voxelize>(voxelize.setup_func, voxelize.render_func, voxelize.flags);
		if (pSSM_Global.setup_func)
			graph.add_library_pass<Passes::PSSM_Global>(pSSM_Global.setup_func, pSSM_Global.render_func, pSSM_Global.flags);
		for (uint32_t i = 0; i < Passes::PSSM_Cascade::MaxCount; ++i)
			if (pSSM_Cascade.setup_funcs[i])
				graph.add_library_pass<Passes::PSSM_Cascade>(Passes::PSSM_Cascade::Names[i], pSSM_Cascade.setup_funcs[i], pSSM_Cascade.render_funcs[i], pSSM_Cascade.flags);
		if (cubeSky.setup_func)
			graph.add_library_pass<Passes::CubeSky>(cubeSky.setup_func, cubeSky.render_func, cubeSky.flags);
		graph.add_library_pass<Passes::CubeMapDownsample>(PassDefault<Passes::CubeMapDownsample>::setup, PassDefault<Passes::CubeMapDownsample>::render, PassDefault<Passes::CubeMapDownsample>::flags);
		graph.add_library_pass<Passes::CubeMapEnviromentProcessor>(PassDefault<Passes::CubeMapEnviromentProcessor>::setup, PassDefault<Passes::CubeMapEnviromentProcessor>::render, PassDefault<Passes::CubeMapEnviromentProcessor>::flags);
		if (lighting.setup_func)
			graph.add_library_pass<Passes::Lighting>(lighting.setup_func, lighting.render_func, lighting.flags);
		if (mipmapping.setup_func)
			graph.add_library_pass<Passes::Mipmapping>(mipmapping.setup_func, mipmapping.render_func, mipmapping.flags);
		if (scene.setup_func)
			graph.add_library_pass<Passes::Scene>(scene.setup_func, scene.render_func, scene.flags);
		if (rTXPass.setup_func)
			graph.add_library_pass<Passes::RTXPass>(rTXPass.setup_func, rTXPass.render_func, rTXPass.flags);
		graph.add_library_pass<Passes::ResultCreation>(PassDefault<Passes::ResultCreation>::setup, PassDefault<Passes::ResultCreation>::render, PassDefault<Passes::ResultCreation>::flags);
		if (pSSM_GenerateMask.setup_func)
			graph.add_library_pass<Passes::PSSM_GenerateMask>(pSSM_GenerateMask.setup_func, pSSM_GenerateMask.render_func, pSSM_GenerateMask.flags);
		if (pSSM_Combine.setup_func)
			graph.add_library_pass<Passes::PSSM_Combine>(pSSM_Combine.setup_func, pSSM_Combine.render_func, pSSM_Combine.flags);
		if (voxelScreen.setup_func)
			graph.add_library_pass<Passes::VoxelScreen>(voxelScreen.setup_func, voxelScreen.render_func, voxelScreen.flags);
		if (voxelCombine.setup_func)
			graph.add_library_pass<Passes::VoxelCombine>(voxelCombine.setup_func, voxelCombine.render_func, voxelCombine.flags);
		if (screenReflection.setup_func)
			graph.add_library_pass<Passes::ScreenReflection>(screenReflection.setup_func, screenReflection.render_func, screenReflection.flags);
		if (reflectionDenoiser_Reproject.setup_func)
			graph.add_library_pass<Passes::ReflectionDenoiser_Reproject>(reflectionDenoiser_Reproject.setup_func, reflectionDenoiser_Reproject.render_func, reflectionDenoiser_Reproject.flags);
		if (reflCombine.setup_func)
			graph.add_library_pass<Passes::ReflCombine>(reflCombine.setup_func, reflCombine.render_func, reflCombine.flags);
		if (voxelDebug.setup_func)
			graph.add_library_pass<Passes::VoxelDebug>(voxelDebug.setup_func, voxelDebug.render_func, voxelDebug.flags);
		if (sky.setup_func)
			graph.add_library_pass<Passes::Sky>(sky.setup_func, sky.render_func, sky.flags);
		if (stencil_renderer_before.setup_func)
			graph.add_library_pass<Passes::stencil_renderer_before>(stencil_renderer_before.setup_func, stencil_renderer_before.render_func, stencil_renderer_before.flags);
		if (stencil_renderer_after.setup_func)
			graph.add_library_pass<Passes::stencil_renderer_after>(stencil_renderer_after.setup_func, stencil_renderer_after.render_func, stencil_renderer_after.flags);
		if (sMAA.setup_func)
			graph.add_library_pass<Passes::SMAA>(sMAA.setup_func, sMAA.render_func, sMAA.flags);
		graph.add_library_pass<Passes::FSR>(PassDefault<Passes::FSR>::setup, PassDefault<Passes::FSR>::render, PassDefault<Passes::FSR>::flags);
		graph.add_library_pass<Passes::CopyPrev>(PassDefault<Passes::CopyPrev>::setup, PassDefault<Passes::CopyPrev>::render, PassDefault<Passes::CopyPrev>::flags);
		graph.add_library_pass<Passes::Profiler>(PassDefault<Passes::Profiler>::setup, PassDefault<Passes::Profiler>::render, PassDefault<Passes::Profiler>::flags);
	}
};

}
