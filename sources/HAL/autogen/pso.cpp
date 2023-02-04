import HAL;
import Core;

import ppl;
using namespace concurrency;

void init_signatures(HAL::Device& device, enum_array<Layouts, HAL::RootLayout::ptr>& signatures)
{
	signatures[Layouts::FrameLayout] = AutoGenSignatureDesc<FrameLayout>().create_signature(device, Layouts::FrameLayout);
	signatures[Layouts::DefaultLayout] = AutoGenSignatureDesc<DefaultLayout>().create_signature(device, Layouts::DefaultLayout);
}
void init_pso(HAL::Device& device, enum_array<PSO, PSOBase::ptr>& pso)
{
	 std::vector<task<void>> tasks;
	tasks.emplace_back(PSOBase::create<PSOS::BRDF>(device, pso[PSO::BRDF]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserShadow_Prepare>(device, pso[PSO::DenoiserShadow_Prepare]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserShadow_TileClassification>(device, pso[PSO::DenoiserShadow_TileClassification]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserShadow_Filter>(device, pso[PSO::DenoiserShadow_Filter]));
	tasks.emplace_back(PSOBase::create<PSOS::FSR>(device, pso[PSO::FSR]));
	tasks.emplace_back(PSOBase::create<PSOS::RCAS>(device, pso[PSO::RCAS]));
	tasks.emplace_back(PSOBase::create<PSOS::GatherPipeline>(device, pso[PSO::GatherPipeline]));
	tasks.emplace_back(PSOBase::create<PSOS::GatherBoxes>(device, pso[PSO::GatherBoxes]));
	tasks.emplace_back(PSOBase::create<PSOS::InitDispatch>(device, pso[PSO::InitDispatch]));
	tasks.emplace_back(PSOBase::create<PSOS::GatherMeshes>(device, pso[PSO::GatherMeshes]));
	tasks.emplace_back(PSOBase::create<PSOS::DownsampleDepth>(device, pso[PSO::DownsampleDepth]));
	tasks.emplace_back(PSOBase::create<PSOS::MipMapping>(device, pso[PSO::MipMapping]));
	tasks.emplace_back(PSOBase::create<PSOS::Lighting>(device, pso[PSO::Lighting]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelDownsample>(device, pso[PSO::VoxelDownsample]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelCopy>(device, pso[PSO::VoxelCopy]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelZero>(device, pso[PSO::VoxelZero]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelVisibility>(device, pso[PSO::VoxelVisibility]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectFilter>(device, pso[PSO::VoxelIndirectFilter]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectLow>(device, pso[PSO::VoxelIndirectLow]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserHistoryFix>(device, pso[PSO::DenoiserHistoryFix]));
	tasks.emplace_back(PSOBase::create<PSOS::FrameClassification>(device, pso[PSO::FrameClassification]));
	tasks.emplace_back(PSOBase::create<PSOS::FrameClassificationInitDispatch>(device, pso[PSO::FrameClassificationInitDispatch]));
	tasks.emplace_back(PSOBase::create<PSOS::FontRender>(device,pso[PSO::FontRender]));
	tasks.emplace_back(PSOBase::create<PSOS::RenderBoxes>(device,pso[PSO::RenderBoxes]));
	tasks.emplace_back(PSOBase::create<PSOS::RenderToDS>(device,pso[PSO::RenderToDS]));
	tasks.emplace_back(PSOBase::create<PSOS::QualityColor>(device,pso[PSO::QualityColor]));
	tasks.emplace_back(PSOBase::create<PSOS::QualityToStencil>(device,pso[PSO::QualityToStencil]));
	tasks.emplace_back(PSOBase::create<PSOS::QualityToStencilREfl>(device,pso[PSO::QualityToStencilREfl]));
	tasks.emplace_back(PSOBase::create<PSOS::CopyTexture>(device,pso[PSO::CopyTexture]));
	tasks.emplace_back(PSOBase::create<PSOS::PSSMMask>(device,pso[PSO::PSSMMask]));
	tasks.emplace_back(PSOBase::create<PSOS::PSSMApply>(device,pso[PSO::PSSMApply]));
	tasks.emplace_back(PSOBase::create<PSOS::GBufferDownsample>(device,pso[PSO::GBufferDownsample]));
	tasks.emplace_back(PSOBase::create<PSOS::Sky>(device,pso[PSO::Sky]));
	tasks.emplace_back(PSOBase::create<PSOS::SkyCube>(device,pso[PSO::SkyCube]));
	tasks.emplace_back(PSOBase::create<PSOS::CubemapENV>(device,pso[PSO::CubemapENV]));
	tasks.emplace_back(PSOBase::create<PSOS::CubemapENVDiffuse>(device,pso[PSO::CubemapENVDiffuse]));
	tasks.emplace_back(PSOBase::create<PSOS::EdgeDetect>(device,pso[PSO::EdgeDetect]));
	tasks.emplace_back(PSOBase::create<PSOS::BlendWeight>(device,pso[PSO::BlendWeight]));
	tasks.emplace_back(PSOBase::create<PSOS::Blending>(device,pso[PSO::Blending]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawStencil>(device,pso[PSO::DrawStencil]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawSelected>(device,pso[PSO::DrawSelected]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawBox>(device,pso[PSO::DrawBox]));
	tasks.emplace_back(PSOBase::create<PSOS::DrawAxis>(device,pso[PSO::DrawAxis]));
	tasks.emplace_back(PSOBase::create<PSOS::StencilerLast>(device,pso[PSO::StencilerLast]));
	tasks.emplace_back(PSOBase::create<PSOS::NinePatch>(device,pso[PSO::NinePatch]));
	tasks.emplace_back(PSOBase::create<PSOS::SimpleRect>(device,pso[PSO::SimpleRect]));
	tasks.emplace_back(PSOBase::create<PSOS::CanvasBack>(device,pso[PSO::CanvasBack]));
	tasks.emplace_back(PSOBase::create<PSOS::CanvasLines>(device,pso[PSO::CanvasLines]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelReflectionHi>(device,pso[PSO::VoxelReflectionHi]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelReflectionUpsample>(device,pso[PSO::VoxelReflectionUpsample]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectHi>(device,pso[PSO::VoxelIndirectHi]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelIndirectUpsample>(device,pso[PSO::VoxelIndirectUpsample]));
	tasks.emplace_back(PSOBase::create<PSOS::VoxelDebug>(device,pso[PSO::VoxelDebug]));
	tasks.emplace_back(PSOBase::create<PSOS::DenoiserDownsample>(device,pso[PSO::DenoiserDownsample]));
	 when_all(begin(tasks), end(tasks)).wait();
}
