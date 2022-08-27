
module;
#include "pch_dx.h"
#include "Graphics/dx12_types.h"
export module Graphics:Autogen;

import :PipelineState;
import Math;
import :Samplers;
import :Buffer;
import :Descriptors;
import :HLSLDescriptors;

import :SIG;
import :RT;
import :Layout;
import :Slots;
import :PSO;
import :RTX;
import :Enums;

import Data;
import :RootSignature;

import Utils;
import serialization;
import :Types;

import :Definitions;
export
{


	#include "layout\FrameLayout.h"
	#include "layout\DefaultLayout.h"
	#include "slots\TextureRenderer.h"
	#include "slots\BRDF.h"
	#include "tables\DebugStruct.h"
	#include "slots\DebugInfo.h"
	#include "slots\FontRendering.h"
	#include "slots\FontRenderingConstants.h"
	#include "tables\Glyph.h"
	#include "slots\FontRenderingGlyphs.h"
	#include "tables\Frustum.h"
	#include "tables\Camera.h"
	#include "slots\FrameInfo.h"
	#include "tables\FSRConstants.h"
	#include "slots\FSR.h"
	#include "slots\MaterialInfo.h"
	#include "tables\mesh_vertex_input.h"
	#include "tables\AABB.h"
	#include "tables\node_data.h"
	#include "slots\MeshInfo.h"
	#include "tables\Meshlet.h"
	#include "tables\MeshletCullData.h"
	#include "tables\MeshInstance.h"
	#include "tables\CommandData.h"
	#include "tables\MeshCommandData.h"
	#include "tables\MaterialCommandData.h"
	#include "slots\GatherPipelineGlobal.h"
	#include "slots\GatherPipeline.h"
	#include "tables\BoxInfo.h"
	#include "slots\GatherBoxes.h"
	#include "slots\DrawBoxes.h"
	#include "slots\InitDispatch.h"
	#include "slots\GatherMeshesBoxes.h"
	#include "slots\MipMapping.h"
	#include "slots\CopyTexture.h"
	#include "slots\DownsampleDepth.h"
	#include "slots\GBuffer.h"
	#include "slots\GBufferDownsample.h"
	#include "slots\GBufferQuality.h"
	#include "slots\PSSMConstants.h"
	#include "slots\PSSMData.h"
	#include "slots\PSSMDataGlobal.h"
	#include "slots\PSSMLighting.h"
	#include "slots\RaytracingRays.h"
	#include "tables\RayCone.h"
	#include "tables\RayPayload.h"
	#include "tables\ShadowPayload.h"
	#include "tables\Triangle.h"
	#include "slots\Raytracing.h"
	#include "slots\SceneData.h"
	#include "slots\SkyData.h"
	#include "slots\SkyFace.h"
	#include "slots\EnvFilter.h"
	#include "slots\EnvSource.h"
	#include "slots\SMAA_Global.h"
	#include "slots\SMAA_Weights.h"
	#include "slots\SMAA_Blend.h"
	#include "slots\Countour.h"
	#include "slots\DrawStencil.h"
	#include "slots\PickerBuffer.h"
	#include "slots\Instance.h"
	#include "slots\Color.h"
	#include "slots\Test.h"
	#include "tables\vertex_input.h"
	#include "slots\NinePatch.h"
	#include "slots\ColorRect.h"
	#include "slots\FlowGraph.h"
	#include "tables\VSLine.h"
	#include "slots\LineRender.h"
	#include "tables\VoxelTilingParams.h"
	#include "slots\VoxelInfo.h"
	#include "slots\Voxelization.h"
	#include "slots\VoxelScreen.h"
	#include "slots\VoxelOutput.h"
	#include "slots\VoxelBlur.h"
	#include "slots\VoxelUpscale.h"
	#include "slots\VoxelVisibility.h"
	#include "slots\VoxelMipMap.h"
	#include "slots\VoxelCopy.h"
	#include "slots\VoxelZero.h"
	#include "slots\VoxelLighting.h"
	#include "slots\VoxelDebug.h"
	#include "slots\DenoiserDownsample.h"
	#include "tables\TilingParams.h"
	#include "slots\DenoiserHistoryFix.h"
	#include "slots\TilingPostprocess.h"
	#include "slots\FrameClassification.h"
	#include "slots\FrameClassificationInitDispatch.h"
	#include "rt\GBuffer.h"
	#include "pso\BRDF.h"
	#include "pso\FSR.h"
	#include "pso\RCAS.h"
	#include "pso\GatherPipeline.h"
	#include "pso\GatherBoxes.h"
	#include "pso\InitDispatch.h"
	#include "pso\GatherMeshes.h"
	#include "pso\DownsampleDepth.h"
	#include "pso\MipMapping.h"
	#include "pso\Lighting.h"
	#include "pso\VoxelDownsample.h"
	#include "pso\VoxelCopy.h"
	#include "pso\VoxelZero.h"
	#include "pso\VoxelVisibility.h"
	#include "pso\VoxelIndirectFilter.h"
	#include "pso\VoxelIndirectLow.h"
	#include "pso\DenoiserHistoryFix.h"
	#include "pso\FrameClassification.h"
	#include "pso\FrameClassificationInitDispatch.h"
	#include "pso\FontRender.h"
	#include "pso\RenderBoxes.h"
	#include "pso\RenderToDS.h"
	#include "pso\QualityColor.h"
	#include "pso\QualityToStencil.h"
	#include "pso\QualityToStencilREfl.h"
	#include "pso\CopyTexture.h"
	#include "pso\PSSMMask.h"
	#include "pso\PSSMApply.h"
	#include "pso\GBufferDownsample.h"
	#include "pso\Sky.h"
	#include "pso\SkyCube.h"
	#include "pso\CubemapENV.h"
	#include "pso\CubemapENVDiffuse.h"
	#include "pso\EdgeDetect.h"
	#include "pso\BlendWeight.h"
	#include "pso\Blending.h"
	#include "pso\DrawStencil.h"
	#include "pso\DrawSelected.h"
	#include "pso\DrawBox.h"
	#include "pso\DrawAxis.h"
	#include "pso\StencilerLast.h"
	#include "pso\NinePatch.h"
	#include "pso\SimpleRect.h"
	#include "pso\CanvasBack.h"
	#include "pso\CanvasLines.h"
	#include "pso\VoxelReflectionLow.h"
	#include "pso\VoxelReflectionHi.h"
	#include "pso\VoxelReflectionUpsample.h"
	#include "pso\VoxelIndirectHi.h"
	#include "pso\VoxelIndirectUpsample.h"
	#include "pso\VoxelDebug.h"
	#include "pso\DenoiserDownsample.h"
	#include "rtx\MainRTX.h"
	void init_signatures();
	Graphics::RootLayout::ptr get_Signature(Layouts id);
	void init_pso(enum_array<PSO, PSOBase::ptr>&);
	std::optional<SlotID> get_slot(std::string_view slot_name);
	UINT get_slot_id(SlotID id);
}
static enum_array<Layouts, Graphics::RootLayout::ptr> signatures;
void init_signatures()
{
	signatures[Layouts::FrameLayout] = AutoGenSignatureDesc<FrameLayout>().create_signature(Layouts::FrameLayout);
	signatures[Layouts::DefaultLayout] = AutoGenSignatureDesc<DefaultLayout>().create_signature(Layouts::DefaultLayout);
}
Graphics::RootLayout::ptr get_Signature(Layouts id)
{
	return signatures[id];
}
std::optional<SlotID> get_slot(std::string_view slot_name)
{
	if(slot_name == "TextureRenderer")
	{
		return SlotID::TextureRenderer;
	}
	if(slot_name == "BRDF")
	{
		return SlotID::BRDF;
	}
	if(slot_name == "DebugInfo")
	{
		return SlotID::DebugInfo;
	}
	if(slot_name == "FontRendering")
	{
		return SlotID::FontRendering;
	}
	if(slot_name == "FontRenderingConstants")
	{
		return SlotID::FontRenderingConstants;
	}
	if(slot_name == "FontRenderingGlyphs")
	{
		return SlotID::FontRenderingGlyphs;
	}
	if(slot_name == "FrameInfo")
	{
		return SlotID::FrameInfo;
	}
	if(slot_name == "FSR")
	{
		return SlotID::FSR;
	}
	if(slot_name == "MaterialInfo")
	{
		return SlotID::MaterialInfo;
	}
	if(slot_name == "MeshInfo")
	{
		return SlotID::MeshInfo;
	}
	if(slot_name == "GatherPipelineGlobal")
	{
		return SlotID::GatherPipelineGlobal;
	}
	if(slot_name == "GatherPipeline")
	{
		return SlotID::GatherPipeline;
	}
	if(slot_name == "GatherBoxes")
	{
		return SlotID::GatherBoxes;
	}
	if(slot_name == "DrawBoxes")
	{
		return SlotID::DrawBoxes;
	}
	if(slot_name == "InitDispatch")
	{
		return SlotID::InitDispatch;
	}
	if(slot_name == "GatherMeshesBoxes")
	{
		return SlotID::GatherMeshesBoxes;
	}
	if(slot_name == "MipMapping")
	{
		return SlotID::MipMapping;
	}
	if(slot_name == "CopyTexture")
	{
		return SlotID::CopyTexture;
	}
	if(slot_name == "DownsampleDepth")
	{
		return SlotID::DownsampleDepth;
	}
	if(slot_name == "GBuffer")
	{
		return SlotID::GBuffer;
	}
	if(slot_name == "GBufferDownsample")
	{
		return SlotID::GBufferDownsample;
	}
	if(slot_name == "GBufferQuality")
	{
		return SlotID::GBufferQuality;
	}
	if(slot_name == "PSSMConstants")
	{
		return SlotID::PSSMConstants;
	}
	if(slot_name == "PSSMData")
	{
		return SlotID::PSSMData;
	}
	if(slot_name == "PSSMDataGlobal")
	{
		return SlotID::PSSMDataGlobal;
	}
	if(slot_name == "PSSMLighting")
	{
		return SlotID::PSSMLighting;
	}
	if(slot_name == "RaytracingRays")
	{
		return SlotID::RaytracingRays;
	}
	if(slot_name == "Raytracing")
	{
		return SlotID::Raytracing;
	}
	if(slot_name == "SceneData")
	{
		return SlotID::SceneData;
	}
	if(slot_name == "SkyData")
	{
		return SlotID::SkyData;
	}
	if(slot_name == "SkyFace")
	{
		return SlotID::SkyFace;
	}
	if(slot_name == "EnvFilter")
	{
		return SlotID::EnvFilter;
	}
	if(slot_name == "EnvSource")
	{
		return SlotID::EnvSource;
	}
	if(slot_name == "SMAA_Global")
	{
		return SlotID::SMAA_Global;
	}
	if(slot_name == "SMAA_Weights")
	{
		return SlotID::SMAA_Weights;
	}
	if(slot_name == "SMAA_Blend")
	{
		return SlotID::SMAA_Blend;
	}
	if(slot_name == "Countour")
	{
		return SlotID::Countour;
	}
	if(slot_name == "DrawStencil")
	{
		return SlotID::DrawStencil;
	}
	if(slot_name == "PickerBuffer")
	{
		return SlotID::PickerBuffer;
	}
	if(slot_name == "Instance")
	{
		return SlotID::Instance;
	}
	if(slot_name == "Color")
	{
		return SlotID::Color;
	}
	if(slot_name == "Test")
	{
		return SlotID::Test;
	}
	if(slot_name == "NinePatch")
	{
		return SlotID::NinePatch;
	}
	if(slot_name == "ColorRect")
	{
		return SlotID::ColorRect;
	}
	if(slot_name == "FlowGraph")
	{
		return SlotID::FlowGraph;
	}
	if(slot_name == "LineRender")
	{
		return SlotID::LineRender;
	}
	if(slot_name == "VoxelInfo")
	{
		return SlotID::VoxelInfo;
	}
	if(slot_name == "Voxelization")
	{
		return SlotID::Voxelization;
	}
	if(slot_name == "VoxelScreen")
	{
		return SlotID::VoxelScreen;
	}
	if(slot_name == "VoxelOutput")
	{
		return SlotID::VoxelOutput;
	}
	if(slot_name == "VoxelBlur")
	{
		return SlotID::VoxelBlur;
	}
	if(slot_name == "VoxelUpscale")
	{
		return SlotID::VoxelUpscale;
	}
	if(slot_name == "VoxelVisibility")
	{
		return SlotID::VoxelVisibility;
	}
	if(slot_name == "VoxelMipMap")
	{
		return SlotID::VoxelMipMap;
	}
	if(slot_name == "VoxelCopy")
	{
		return SlotID::VoxelCopy;
	}
	if(slot_name == "VoxelZero")
	{
		return SlotID::VoxelZero;
	}
	if(slot_name == "VoxelLighting")
	{
		return SlotID::VoxelLighting;
	}
	if(slot_name == "VoxelDebug")
	{
		return SlotID::VoxelDebug;
	}
	if(slot_name == "DenoiserDownsample")
	{
		return SlotID::DenoiserDownsample;
	}
	if(slot_name == "DenoiserHistoryFix")
	{
		return SlotID::DenoiserHistoryFix;
	}
	if(slot_name == "TilingPostprocess")
	{
		return SlotID::TilingPostprocess;
	}
	if(slot_name == "FrameClassification")
	{
		return SlotID::FrameClassification;
	}
	if(slot_name == "FrameClassificationInitDispatch")
	{
		return SlotID::FrameClassificationInitDispatch;
	}
	return std::nullopt;
}
UINT get_slot_id(SlotID id)
{
	if(id == SlotID::TextureRenderer)
	{
		return Slots::TextureRenderer::Slot::ID;
	}
	if(id == SlotID::BRDF)
	{
		return Slots::BRDF::Slot::ID;
	}
	if(id == SlotID::DebugInfo)
	{
		return Slots::DebugInfo::Slot::ID;
	}
	if(id == SlotID::FontRendering)
	{
		return Slots::FontRendering::Slot::ID;
	}
	if(id == SlotID::FontRenderingConstants)
	{
		return Slots::FontRenderingConstants::Slot::ID;
	}
	if(id == SlotID::FontRenderingGlyphs)
	{
		return Slots::FontRenderingGlyphs::Slot::ID;
	}
	if(id == SlotID::FrameInfo)
	{
		return Slots::FrameInfo::Slot::ID;
	}
	if(id == SlotID::FSR)
	{
		return Slots::FSR::Slot::ID;
	}
	if(id == SlotID::MaterialInfo)
	{
		return Slots::MaterialInfo::Slot::ID;
	}
	if(id == SlotID::MeshInfo)
	{
		return Slots::MeshInfo::Slot::ID;
	}
	if(id == SlotID::GatherPipelineGlobal)
	{
		return Slots::GatherPipelineGlobal::Slot::ID;
	}
	if(id == SlotID::GatherPipeline)
	{
		return Slots::GatherPipeline::Slot::ID;
	}
	if(id == SlotID::GatherBoxes)
	{
		return Slots::GatherBoxes::Slot::ID;
	}
	if(id == SlotID::DrawBoxes)
	{
		return Slots::DrawBoxes::Slot::ID;
	}
	if(id == SlotID::InitDispatch)
	{
		return Slots::InitDispatch::Slot::ID;
	}
	if(id == SlotID::GatherMeshesBoxes)
	{
		return Slots::GatherMeshesBoxes::Slot::ID;
	}
	if(id == SlotID::MipMapping)
	{
		return Slots::MipMapping::Slot::ID;
	}
	if(id == SlotID::CopyTexture)
	{
		return Slots::CopyTexture::Slot::ID;
	}
	if(id == SlotID::DownsampleDepth)
	{
		return Slots::DownsampleDepth::Slot::ID;
	}
	if(id == SlotID::GBuffer)
	{
		return Slots::GBuffer::Slot::ID;
	}
	if(id == SlotID::GBufferDownsample)
	{
		return Slots::GBufferDownsample::Slot::ID;
	}
	if(id == SlotID::GBufferQuality)
	{
		return Slots::GBufferQuality::Slot::ID;
	}
	if(id == SlotID::PSSMConstants)
	{
		return Slots::PSSMConstants::Slot::ID;
	}
	if(id == SlotID::PSSMData)
	{
		return Slots::PSSMData::Slot::ID;
	}
	if(id == SlotID::PSSMDataGlobal)
	{
		return Slots::PSSMDataGlobal::Slot::ID;
	}
	if(id == SlotID::PSSMLighting)
	{
		return Slots::PSSMLighting::Slot::ID;
	}
	if(id == SlotID::RaytracingRays)
	{
		return Slots::RaytracingRays::Slot::ID;
	}
	if(id == SlotID::Raytracing)
	{
		return Slots::Raytracing::Slot::ID;
	}
	if(id == SlotID::SceneData)
	{
		return Slots::SceneData::Slot::ID;
	}
	if(id == SlotID::SkyData)
	{
		return Slots::SkyData::Slot::ID;
	}
	if(id == SlotID::SkyFace)
	{
		return Slots::SkyFace::Slot::ID;
	}
	if(id == SlotID::EnvFilter)
	{
		return Slots::EnvFilter::Slot::ID;
	}
	if(id == SlotID::EnvSource)
	{
		return Slots::EnvSource::Slot::ID;
	}
	if(id == SlotID::SMAA_Global)
	{
		return Slots::SMAA_Global::Slot::ID;
	}
	if(id == SlotID::SMAA_Weights)
	{
		return Slots::SMAA_Weights::Slot::ID;
	}
	if(id == SlotID::SMAA_Blend)
	{
		return Slots::SMAA_Blend::Slot::ID;
	}
	if(id == SlotID::Countour)
	{
		return Slots::Countour::Slot::ID;
	}
	if(id == SlotID::DrawStencil)
	{
		return Slots::DrawStencil::Slot::ID;
	}
	if(id == SlotID::PickerBuffer)
	{
		return Slots::PickerBuffer::Slot::ID;
	}
	if(id == SlotID::Instance)
	{
		return Slots::Instance::Slot::ID;
	}
	if(id == SlotID::Color)
	{
		return Slots::Color::Slot::ID;
	}
	if(id == SlotID::Test)
	{
		return Slots::Test::Slot::ID;
	}
	if(id == SlotID::NinePatch)
	{
		return Slots::NinePatch::Slot::ID;
	}
	if(id == SlotID::ColorRect)
	{
		return Slots::ColorRect::Slot::ID;
	}
	if(id == SlotID::FlowGraph)
	{
		return Slots::FlowGraph::Slot::ID;
	}
	if(id == SlotID::LineRender)
	{
		return Slots::LineRender::Slot::ID;
	}
	if(id == SlotID::VoxelInfo)
	{
		return Slots::VoxelInfo::Slot::ID;
	}
	if(id == SlotID::Voxelization)
	{
		return Slots::Voxelization::Slot::ID;
	}
	if(id == SlotID::VoxelScreen)
	{
		return Slots::VoxelScreen::Slot::ID;
	}
	if(id == SlotID::VoxelOutput)
	{
		return Slots::VoxelOutput::Slot::ID;
	}
	if(id == SlotID::VoxelBlur)
	{
		return Slots::VoxelBlur::Slot::ID;
	}
	if(id == SlotID::VoxelUpscale)
	{
		return Slots::VoxelUpscale::Slot::ID;
	}
	if(id == SlotID::VoxelVisibility)
	{
		return Slots::VoxelVisibility::Slot::ID;
	}
	if(id == SlotID::VoxelMipMap)
	{
		return Slots::VoxelMipMap::Slot::ID;
	}
	if(id == SlotID::VoxelCopy)
	{
		return Slots::VoxelCopy::Slot::ID;
	}
	if(id == SlotID::VoxelZero)
	{
		return Slots::VoxelZero::Slot::ID;
	}
	if(id == SlotID::VoxelLighting)
	{
		return Slots::VoxelLighting::Slot::ID;
	}
	if(id == SlotID::VoxelDebug)
	{
		return Slots::VoxelDebug::Slot::ID;
	}
	if(id == SlotID::DenoiserDownsample)
	{
		return Slots::DenoiserDownsample::Slot::ID;
	}
	if(id == SlotID::DenoiserHistoryFix)
	{
		return Slots::DenoiserHistoryFix::Slot::ID;
	}
	if(id == SlotID::TilingPostprocess)
	{
		return Slots::TilingPostprocess::Slot::ID;
	}
	if(id == SlotID::FrameClassification)
	{
		return Slots::FrameClassification::Slot::ID;
	}
	if(id == SlotID::FrameClassificationInitDispatch)
	{
		return Slots::FrameClassificationInitDispatch::Slot::ID;
	}
	return -1;
}
