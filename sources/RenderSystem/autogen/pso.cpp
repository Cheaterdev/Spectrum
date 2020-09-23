#include "pch.h"
void init_pso(std::array<PSOBase::ptr, int(PSO::TOTAL)>& pso)
{
	pso[static_cast<int>(PSO::BRDF)] =  std::make_shared<PSOS::BRDF>();
	pso[static_cast<int>(PSO::GatherPipeline)] =  std::make_shared<PSOS::GatherPipeline>();
	pso[static_cast<int>(PSO::GatherBoxes)] =  std::make_shared<PSOS::GatherBoxes>();
	pso[static_cast<int>(PSO::InitDispatch)] =  std::make_shared<PSOS::InitDispatch>();
	pso[static_cast<int>(PSO::GatherMeshes)] =  std::make_shared<PSOS::GatherMeshes>();
	pso[static_cast<int>(PSO::DownsampleDepth)] =  std::make_shared<PSOS::DownsampleDepth>();
	pso[static_cast<int>(PSO::MipMapping)] =  std::make_shared<PSOS::MipMapping>();
	pso[static_cast<int>(PSO::Lighting)] =  std::make_shared<PSOS::Lighting>();
	pso[static_cast<int>(PSO::VoxelDownsample)] =  std::make_shared<PSOS::VoxelDownsample>();
	pso[static_cast<int>(PSO::FontRender)] =  std::make_shared<PSOS::FontRender>();
	pso[static_cast<int>(PSO::FontRenderClip)] =  std::make_shared<PSOS::FontRenderClip>();
	pso[static_cast<int>(PSO::RenderBoxes)] =  std::make_shared<PSOS::RenderBoxes>();
	pso[static_cast<int>(PSO::RenderToDS)] =  std::make_shared<PSOS::RenderToDS>();
	pso[static_cast<int>(PSO::QualityColor)] =  std::make_shared<PSOS::QualityColor>();
	pso[static_cast<int>(PSO::QualityToStencil)] =  std::make_shared<PSOS::QualityToStencil>();
	pso[static_cast<int>(PSO::QualityToStencilREfl)] =  std::make_shared<PSOS::QualityToStencilREfl>();
	pso[static_cast<int>(PSO::PSSMMask)] =  std::make_shared<PSOS::PSSMMask>();
	pso[static_cast<int>(PSO::PSSMApply)] =  std::make_shared<PSOS::PSSMApply>();
	pso[static_cast<int>(PSO::GBufferDownsample)] =  std::make_shared<PSOS::GBufferDownsample>();
	pso[static_cast<int>(PSO::Sky)] =  std::make_shared<PSOS::Sky>();
	pso[static_cast<int>(PSO::SkyCube)] =  std::make_shared<PSOS::SkyCube>();
	pso[static_cast<int>(PSO::CubemapENV)] =  std::make_shared<PSOS::CubemapENV>();
	pso[static_cast<int>(PSO::CubemapENVDiffuse)] =  std::make_shared<PSOS::CubemapENVDiffuse>();
	pso[static_cast<int>(PSO::EdgeDetect)] =  std::make_shared<PSOS::EdgeDetect>();
	pso[static_cast<int>(PSO::BlendWeight)] =  std::make_shared<PSOS::BlendWeight>();
	pso[static_cast<int>(PSO::Blending)] =  std::make_shared<PSOS::Blending>();
	pso[static_cast<int>(PSO::DrawStencil)] =  std::make_shared<PSOS::DrawStencil>();
	pso[static_cast<int>(PSO::DrawSelected)] =  std::make_shared<PSOS::DrawSelected>();
	pso[static_cast<int>(PSO::DrawBox)] =  std::make_shared<PSOS::DrawBox>();
	pso[static_cast<int>(PSO::DrawAxis)] =  std::make_shared<PSOS::DrawAxis>();
	pso[static_cast<int>(PSO::StencilerLast)] =  std::make_shared<PSOS::StencilerLast>();
	pso[static_cast<int>(PSO::NinePatch)] =  std::make_shared<PSOS::NinePatch>();
	pso[static_cast<int>(PSO::SimpleRect)] =  std::make_shared<PSOS::SimpleRect>();
	pso[static_cast<int>(PSO::CanvasBack)] =  std::make_shared<PSOS::CanvasBack>();
	pso[static_cast<int>(PSO::CanvasLines)] =  std::make_shared<PSOS::CanvasLines>();
	pso[static_cast<int>(PSO::VoxelReflectionLow)] =  std::make_shared<PSOS::VoxelReflectionLow>();
	pso[static_cast<int>(PSO::VoxelIndirectFilter)] =  std::make_shared<PSOS::VoxelIndirectFilter>();
	pso[static_cast<int>(PSO::VoxelReflectionHi)] =  std::make_shared<PSOS::VoxelReflectionHi>();
	pso[static_cast<int>(PSO::VoxelReflectionUpsample)] =  std::make_shared<PSOS::VoxelReflectionUpsample>();
	pso[static_cast<int>(PSO::VoxelIndirectHi)] =  std::make_shared<PSOS::VoxelIndirectHi>();
	pso[static_cast<int>(PSO::VoxelIndirectLow)] =  std::make_shared<PSOS::VoxelIndirectLow>();
	pso[static_cast<int>(PSO::VoxelIndirectUpsample)] =  std::make_shared<PSOS::VoxelIndirectUpsample>();
	pso[static_cast<int>(PSO::VoxelDebug)] =  std::make_shared<PSOS::VoxelDebug>();
}
