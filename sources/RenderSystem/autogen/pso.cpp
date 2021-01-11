#include "pch.h"
#include <slots.h>
void init_pso(enum_array<PSO, PSOBase::ptr>& pso)
{
	pso[PSO::BRDF] =  std::make_shared<PSOS::BRDF>();
	pso[PSO::GatherPipeline] =  std::make_shared<PSOS::GatherPipeline>();
	pso[PSO::GatherBoxes] =  std::make_shared<PSOS::GatherBoxes>();
	pso[PSO::InitDispatch] =  std::make_shared<PSOS::InitDispatch>();
	pso[PSO::GatherMeshes] =  std::make_shared<PSOS::GatherMeshes>();
	pso[PSO::DownsampleDepth] =  std::make_shared<PSOS::DownsampleDepth>();
	pso[PSO::MipMapping] =  std::make_shared<PSOS::MipMapping>();
	pso[PSO::Lighting] =  std::make_shared<PSOS::Lighting>();
	pso[PSO::VoxelDownsample] =  std::make_shared<PSOS::VoxelDownsample>();
	pso[PSO::VoxelCopy] =  std::make_shared<PSOS::VoxelCopy>();
	pso[PSO::VoxelZero] =  std::make_shared<PSOS::VoxelZero>();
	pso[PSO::VoxelVisibility] =  std::make_shared<PSOS::VoxelVisibility>();
	pso[PSO::FontRender] =  std::make_shared<PSOS::FontRender>();
	pso[PSO::RenderBoxes] =  std::make_shared<PSOS::RenderBoxes>();
	pso[PSO::RenderToDS] =  std::make_shared<PSOS::RenderToDS>();
	pso[PSO::QualityColor] =  std::make_shared<PSOS::QualityColor>();
	pso[PSO::QualityToStencil] =  std::make_shared<PSOS::QualityToStencil>();
	pso[PSO::QualityToStencilREfl] =  std::make_shared<PSOS::QualityToStencilREfl>();
	pso[PSO::CopyTexture] =  std::make_shared<PSOS::CopyTexture>();
	pso[PSO::PSSMMask] =  std::make_shared<PSOS::PSSMMask>();
	pso[PSO::PSSMApply] =  std::make_shared<PSOS::PSSMApply>();
	pso[PSO::GBufferDownsample] =  std::make_shared<PSOS::GBufferDownsample>();
	pso[PSO::Sky] =  std::make_shared<PSOS::Sky>();
	pso[PSO::SkyCube] =  std::make_shared<PSOS::SkyCube>();
	pso[PSO::CubemapENV] =  std::make_shared<PSOS::CubemapENV>();
	pso[PSO::CubemapENVDiffuse] =  std::make_shared<PSOS::CubemapENVDiffuse>();
	pso[PSO::EdgeDetect] =  std::make_shared<PSOS::EdgeDetect>();
	pso[PSO::BlendWeight] =  std::make_shared<PSOS::BlendWeight>();
	pso[PSO::Blending] =  std::make_shared<PSOS::Blending>();
	pso[PSO::DrawStencil] =  std::make_shared<PSOS::DrawStencil>();
	pso[PSO::DrawSelected] =  std::make_shared<PSOS::DrawSelected>();
	pso[PSO::DrawBox] =  std::make_shared<PSOS::DrawBox>();
	pso[PSO::DrawAxis] =  std::make_shared<PSOS::DrawAxis>();
	pso[PSO::StencilerLast] =  std::make_shared<PSOS::StencilerLast>();
	pso[PSO::NinePatch] =  std::make_shared<PSOS::NinePatch>();
	pso[PSO::SimpleRect] =  std::make_shared<PSOS::SimpleRect>();
	pso[PSO::CanvasBack] =  std::make_shared<PSOS::CanvasBack>();
	pso[PSO::CanvasLines] =  std::make_shared<PSOS::CanvasLines>();
	pso[PSO::VoxelReflectionLow] =  std::make_shared<PSOS::VoxelReflectionLow>();
	pso[PSO::VoxelIndirectFilter] =  std::make_shared<PSOS::VoxelIndirectFilter>();
	pso[PSO::VoxelReflectionHi] =  std::make_shared<PSOS::VoxelReflectionHi>();
	pso[PSO::VoxelReflectionUpsample] =  std::make_shared<PSOS::VoxelReflectionUpsample>();
	pso[PSO::VoxelIndirectHi] =  std::make_shared<PSOS::VoxelIndirectHi>();
	pso[PSO::VoxelIndirectLow] =  std::make_shared<PSOS::VoxelIndirectLow>();
	pso[PSO::VoxelIndirectUpsample] =  std::make_shared<PSOS::VoxelIndirectUpsample>();
	pso[PSO::VoxelDebug] =  std::make_shared<PSOS::VoxelDebug>();
}
