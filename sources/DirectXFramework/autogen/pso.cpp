#include "pch_dx.h"

import Autogen;
import Data;
import SIG;
import RT;
import Layout;
import Slots;
import PSO;
import RTX;
import Enums;
import RootSignature;

import ppl;
using namespace concurrency;
void init_pso(enum_array<PSO, PSOBase::ptr>& pso)
{
	 std::vector<task<void>> tasks;
	tasks.emplace_back(PSOBase::create<PSOS::BRDF>(pso[PSO::BRDF]));
	tasks.emplace_back(PSOBase::create<PSOS::FSR>(pso[PSO::FSR]));
	tasks.emplace_back(PSOBase::create<PSOS::RCAS>(pso[PSO::RCAS]));
	tasks.emplace_back(PSOBase::create<PSOS::GatherPipeline>(pso[PSO::GatherPipeline]));
	tasks.emplace_back(PSOBase::create<PSOS::GatherBoxes>(pso[PSO::GatherBoxes]));
	tasks.emplace_back(PSOBase::create<PSOS::InitDispatch>(pso[PSO::InitDispatch]));
	tasks.emplace_back(PSOBase::create<PSOS::GatherMeshes>(pso[PSO::GatherMeshes]));
	tasks.emplace_back(PSOBase::create<PSOS::DownsampleDepth>(pso[PSO::DownsampleDepth]));
	tasks.emplace_back(PSOBase::create<PSOS::MipMapping>(pso[PSO::MipMapping]));
	tasks.emplace_back(PSOBase::create<PSOS::Lighting>(pso[PSO::Lighting]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelDownsample>(pso[PSO::VoxelDownsample]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelCopy>(pso[PSO::VoxelCopy]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelZero>(pso[PSO::VoxelZero]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelVisibility>(pso[PSO::VoxelVisibility]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectFilter>(pso[PSO::VoxelIndirectFilter]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectLow>(pso[PSO::VoxelIndirectLow]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserHistoryFix>(pso[PSO::DenoiserHistoryFix]));
	tasks.emplace_back(PSOBase::create<PSOS::FrameClassification>(pso[PSO::FrameClassification]));
	tasks.emplace_back(PSOBase::create<PSOS::FrameClassificationInitDispatch>(pso[PSO::FrameClassificationInitDispatch]));
	tasks.emplace_back(PSOBase::create<PSOS::FontRender>(pso[PSO::FontRender]));
	tasks.emplace_back(PSOBase::create<PSOS::RenderBoxes>(pso[PSO::RenderBoxes]));
	tasks.emplace_back(PSOBase::create<PSOS::RenderToDS>(pso[PSO::RenderToDS]));
	tasks.emplace_back(PSOBase::create<PSOS::QualityColor>(pso[PSO::QualityColor]));
	tasks.emplace_back(PSOBase::create<PSOS::QualityToStencil>(pso[PSO::QualityToStencil]));
	tasks.emplace_back(PSOBase::create<PSOS::QualityToStencilREfl>(pso[PSO::QualityToStencilREfl]));
	tasks.emplace_back(PSOBase::create<PSOS::CopyTexture>(pso[PSO::CopyTexture]));
	tasks.emplace_back(PSOBase::create<PSOS::PSSMMask>(pso[PSO::PSSMMask]));
	tasks.emplace_back(PSOBase::create<PSOS::PSSMApply>(pso[PSO::PSSMApply]));
	tasks.emplace_back(PSOBase::create<PSOS::GBufferDownsample>(pso[PSO::GBufferDownsample]));
	tasks.emplace_back(PSOBase::create<PSOS::Sky>(pso[PSO::Sky]));
	tasks.emplace_back(PSOBase::create<PSOS::SkyCube>(pso[PSO::SkyCube]));
	tasks.emplace_back(PSOBase::create<PSOS::CubemapENV>(pso[PSO::CubemapENV]));
	tasks.emplace_back(PSOBase::create<PSOS::CubemapENVDiffuse>(pso[PSO::CubemapENVDiffuse]));
	tasks.emplace_back(PSOBase::create<PSOS::EdgeDetect>(pso[PSO::EdgeDetect]));
	tasks.emplace_back(PSOBase::create<PSOS::BlendWeight>(pso[PSO::BlendWeight]));
	tasks.emplace_back(PSOBase::create<PSOS::Blending>(pso[PSO::Blending]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawStencil>(pso[PSO::DrawStencil]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawSelected>(pso[PSO::DrawSelected]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawBox>(pso[PSO::DrawBox]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawAxis>(pso[PSO::DrawAxis]));
	tasks.emplace_back(PSOBase::create<PSOS::StencilerLast>(pso[PSO::StencilerLast]));
	tasks.emplace_back(PSOBase::create<PSOS::NinePatch>(pso[PSO::NinePatch]));
	tasks.emplace_back(PSOBase::create<PSOS::SimpleRect>(pso[PSO::SimpleRect]));
	tasks.emplace_back(PSOBase::create<PSOS::CanvasBack>(pso[PSO::CanvasBack]));
	tasks.emplace_back(PSOBase::create<PSOS::CanvasLines>(pso[PSO::CanvasLines]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelReflectionLow>(pso[PSO::VoxelReflectionLow]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelReflectionHi>(pso[PSO::VoxelReflectionHi]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelReflectionUpsample>(pso[PSO::VoxelReflectionUpsample]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectHi>(pso[PSO::VoxelIndirectHi]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectUpsample>(pso[PSO::VoxelIndirectUpsample]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelDebug>(pso[PSO::VoxelDebug]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserDownsample>(pso[PSO::DenoiserDownsample]));
	 when_all(begin(tasks), end(tasks)).wait();
}
