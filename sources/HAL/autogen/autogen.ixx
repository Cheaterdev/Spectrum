
export module HAL:Autogen;

import Core;

import :PipelineState;
import :SIG;
import :RT;
import :Layout;
import :Slots;
import :PSO;
import :RTX;
import :Enums;
import :RootSignature;
import :Types;


	export import :Autogen.Layouts.FrameLayout;
	export import :Autogen.Layouts.DefaultLayout;
	export import :Autogen.Slots.TextureRenderer;
	export import :Autogen.Tables.TextureRenderer;
	export import :Autogen.Slots.BlueNoise;
	export import :Autogen.Tables.BlueNoise;
	export import :Autogen.Slots.BRDF;
	export import :Autogen.Tables.BRDF;
	export import :Autogen.Tables.DebugStruct;
	export import :Autogen.Slots.DebugInfo;
	export import :Autogen.Tables.DebugInfo;
	export import :Autogen.Slots.DenoiserReflectionCommon;
	export import :Autogen.Tables.DenoiserReflectionCommon;
	export import :Autogen.Slots.DenoiserReflectionReproject;
	export import :Autogen.Tables.DenoiserReflectionReproject;
	export import :Autogen.Slots.DenoiserReflectionPrefilter;
	export import :Autogen.Tables.DenoiserReflectionPrefilter;
	export import :Autogen.Slots.DenoiserReflectionResolve;
	export import :Autogen.Tables.DenoiserReflectionResolve;
	export import :Autogen.Slots.DenoiserShadow_Prepare;
	export import :Autogen.Tables.DenoiserShadow_Prepare;
	export import :Autogen.Slots.DenoiserShadow_TileClassification;
	export import :Autogen.Tables.DenoiserShadow_TileClassification;
	export import :Autogen.Slots.DenoiserShadow_Filter;
	export import :Autogen.Tables.DenoiserShadow_Filter;
	export import :Autogen.Slots.DenoiserShadow_FilterLocal;
	export import :Autogen.Tables.DenoiserShadow_FilterLocal;
	export import :Autogen.Slots.DenoiserShadow_FilterLast;
	export import :Autogen.Tables.DenoiserShadow_FilterLast;
	export import :Autogen.Slots.FontRendering;
	export import :Autogen.Tables.FontRendering;
	export import :Autogen.Slots.FontRenderingConstants;
	export import :Autogen.Tables.FontRenderingConstants;
	export import :Autogen.Tables.Glyph;
	export import :Autogen.Slots.FontRenderingGlyphs;
	export import :Autogen.Tables.FontRenderingGlyphs;
	export import :Autogen.Tables.Frustum;
	export import :Autogen.Tables.Camera;
	export import :Autogen.Slots.FrameInfo;
	export import :Autogen.Tables.FrameInfo;
	export import :Autogen.Tables.FSRConstants;
	export import :Autogen.Slots.FSR;
	export import :Autogen.Tables.FSR;
	export import :Autogen.Tables.SingleColor;
	export import :Autogen.Tables.NoOutput;
	export import :Autogen.Tables.DepthOnly;
	export import :Autogen.Tables.SingleColorDepth;
	export import :Autogen.Slots.MaterialInfo;
	export import :Autogen.Tables.MaterialInfo;
	export import :Autogen.Tables.mesh_vertex_input;
	export import :Autogen.Tables.AABB;
	export import :Autogen.Tables.node_data;
	export import :Autogen.Slots.MeshInstanceInfo;
	export import :Autogen.Tables.MeshInstanceInfo;
	export import :Autogen.Slots.MeshInfo;
	export import :Autogen.Tables.MeshInfo;
	export import :Autogen.Tables.RaytraceInstanceInfo;
	export import :Autogen.Tables.Meshlet;
	export import :Autogen.Tables.MeshletCullData;
	export import :Autogen.Tables.MeshInstance;
	export import :Autogen.Tables.CommandData;
	export import :Autogen.Tables.MeshCommandData;
	export import :Autogen.Tables.MaterialCommandData;
	export import :Autogen.Slots.GatherPipelineGlobal;
	export import :Autogen.Tables.GatherPipelineGlobal;
	export import :Autogen.Slots.GatherPipeline;
	export import :Autogen.Tables.GatherPipeline;
	export import :Autogen.Tables.BoxInfo;
	export import :Autogen.Slots.GatherBoxes;
	export import :Autogen.Tables.GatherBoxes;
	export import :Autogen.Slots.DrawBoxes;
	export import :Autogen.Tables.DrawBoxes;
	export import :Autogen.Slots.InitDispatch;
	export import :Autogen.Tables.InitDispatch;
	export import :Autogen.Slots.GatherMeshesBoxes;
	export import :Autogen.Tables.GatherMeshesBoxes;
	export import :Autogen.Slots.MipMapping;
	export import :Autogen.Tables.MipMapping;
	export import :Autogen.Slots.CopyTexture;
	export import :Autogen.Tables.CopyTexture;
	export import :Autogen.Slots.DownsampleDepth;
	export import :Autogen.Tables.DownsampleDepth;
	export import :Autogen.Slots.GBufferDownsample;
	export import :Autogen.Tables.GBufferDownsample;
	export import :Autogen.Slots.GBufferQuality;
	export import :Autogen.Tables.GBufferQuality;
	export import :Autogen.Slots.PSSMConstants;
	export import :Autogen.Tables.PSSMConstants;
	export import :Autogen.Slots.PSSMData;
	export import :Autogen.Tables.PSSMData;
	export import :Autogen.Slots.PSSMDataGlobal;
	export import :Autogen.Tables.PSSMDataGlobal;
	export import :Autogen.Slots.PSSMLighting;
	export import :Autogen.Tables.PSSMLighting;
	export import :Autogen.Tables.GBufferDownsampleRT;
	export import :Autogen.Slots.RaytracingRays;
	export import :Autogen.Tables.RaytracingRays;
	export import :Autogen.Tables.RayCone;
	export import :Autogen.Tables.RayPayload;
	export import :Autogen.Tables.ShadowPayload;
	export import :Autogen.Tables.Triangle;
	export import :Autogen.Slots.Raytracing;
	export import :Autogen.Tables.Raytracing;
	export import :Autogen.Slots.SceneData;
	export import :Autogen.Tables.SceneData;
	export import :Autogen.Slots.GBuffer;
	export import :Autogen.Tables.GBuffer;
	export import :Autogen.Slots.SkyData;
	export import :Autogen.Tables.SkyData;
	export import :Autogen.Slots.SkyFace;
	export import :Autogen.Tables.SkyFace;
	export import :Autogen.Slots.EnvFilter;
	export import :Autogen.Tables.EnvFilter;
	export import :Autogen.Slots.EnvSource;
	export import :Autogen.Tables.EnvSource;
	export import :Autogen.Slots.SMAA_Global;
	export import :Autogen.Tables.SMAA_Global;
	export import :Autogen.Slots.SMAA_Weights;
	export import :Autogen.Tables.SMAA_Weights;
	export import :Autogen.Slots.SMAA_Blend;
	export import :Autogen.Tables.SMAA_Blend;
	export import :Autogen.Slots.Countour;
	export import :Autogen.Tables.Countour;
	export import :Autogen.Slots.DrawStencil;
	export import :Autogen.Tables.DrawStencil;
	export import :Autogen.Slots.PickerBuffer;
	export import :Autogen.Tables.PickerBuffer;
	export import :Autogen.Slots.Instance;
	export import :Autogen.Tables.Instance;
	export import :Autogen.Slots.Color;
	export import :Autogen.Tables.Color;
	export import :Autogen.Slots.Test;
	export import :Autogen.Tables.Test;
	export import :Autogen.Tables.vertex_input;
	export import :Autogen.Slots.NinePatch;
	export import :Autogen.Tables.NinePatch;
	export import :Autogen.Slots.ColorRect;
	export import :Autogen.Tables.ColorRect;
	export import :Autogen.Slots.FlowGraph;
	export import :Autogen.Tables.FlowGraph;
	export import :Autogen.Tables.VSLine;
	export import :Autogen.Slots.LineRender;
	export import :Autogen.Tables.LineRender;
	export import :Autogen.Tables.VoxelTilingParams;
	export import :Autogen.Slots.VoxelInfo;
	export import :Autogen.Tables.VoxelInfo;
	export import :Autogen.Slots.Voxelization;
	export import :Autogen.Tables.Voxelization;
	export import :Autogen.Slots.VoxelScreen;
	export import :Autogen.Tables.VoxelScreen;
	export import :Autogen.Slots.VoxelOutput;
	export import :Autogen.Tables.VoxelOutput;
	export import :Autogen.Slots.VoxelBlur;
	export import :Autogen.Tables.VoxelBlur;
	export import :Autogen.Slots.VoxelUpscale;
	export import :Autogen.Tables.VoxelUpscale;
	export import :Autogen.Slots.VoxelVisibility;
	export import :Autogen.Tables.VoxelVisibility;
	export import :Autogen.Slots.VoxelMipMap;
	export import :Autogen.Tables.VoxelMipMap;
	export import :Autogen.Slots.VoxelCopy;
	export import :Autogen.Tables.VoxelCopy;
	export import :Autogen.Slots.VoxelZero;
	export import :Autogen.Tables.VoxelZero;
	export import :Autogen.Slots.VoxelLighting;
	export import :Autogen.Tables.VoxelLighting;
	export import :Autogen.Slots.VoxelDebug;
	export import :Autogen.Tables.VoxelDebug;
	export import :Autogen.Slots.DenoiserDownsample;
	export import :Autogen.Tables.DenoiserDownsample;
	export import :Autogen.Tables.TilingParams;
	export import :Autogen.Slots.DenoiserHistoryFix;
	export import :Autogen.Tables.DenoiserHistoryFix;
	export import :Autogen.Slots.TilingPostprocess;
	export import :Autogen.Tables.TilingPostprocess;
	export import :Autogen.Slots.FrameClassification;
	export import :Autogen.Tables.FrameClassification;
	export import :Autogen.Slots.FrameClassificationInitDispatch;
	export import :Autogen.Tables.FrameClassificationInitDispatch;
	export import :Autogen.Slots.ReflectionCombine;
	export import :Autogen.Tables.ReflectionCombine;
	export{
	#include "rt\SingleColor.h"
	#include "rt\NoOutput.h"
	#include "rt\DepthOnly.h"
	#include "rt\SingleColorDepth.h"
	#include "rt\GBufferDownsampleRT.h"
	#include "rt\GBuffer.h"
	#include "pso\BlueNoise.h"
	#include "pso\BRDF.h"
	#include "pso\DenoiserReflectionReproject.h"
	#include "pso\DenoiserReflectionPrefilter.h"
	#include "pso\DenoiserReflectionResolve.h"
	#include "pso\DenoiserShadow_Prepare.h"
	#include "pso\DenoiserShadow_TileClassification.h"
	#include "pso\DenoiserShadow_Filter.h"
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
	#include "pso\ReflectionCombine.h"
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
	#include "pso\GBufferDraw.h"
	#include "pso\DepthDraw.h"
	#include "pso\Voxelization.h"
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
	#include "pso\VoxelReflectionHi.h"
	#include "pso\VoxelReflectionUpsample.h"
	#include "pso\VoxelIndirectHi.h"
	#include "pso\VoxelIndirectUpsample.h"
	#include "pso\VoxelDebug.h"
	#include "pso\DenoiserDownsample.h"
	#include "rtx\MainRTX.h"
	std::optional<SlotID> get_slot(std::string_view slot_name);
	uint get_table_index(SlotID id);
	std::string get_slot_name(SlotID id);
}
std::optional<SlotID> get_slot(std::string_view slot_name)
{
	if(slot_name == "TextureRenderer")
	{
		return SlotID::TextureRenderer;
	}
	if(slot_name == "BlueNoise")
	{
		return SlotID::BlueNoise;
	}
	if(slot_name == "BRDF")
	{
		return SlotID::BRDF;
	}
	if(slot_name == "DebugInfo")
	{
		return SlotID::DebugInfo;
	}
	if(slot_name == "DenoiserReflectionCommon")
	{
		return SlotID::DenoiserReflectionCommon;
	}
	if(slot_name == "DenoiserReflectionReproject")
	{
		return SlotID::DenoiserReflectionReproject;
	}
	if(slot_name == "DenoiserReflectionPrefilter")
	{
		return SlotID::DenoiserReflectionPrefilter;
	}
	if(slot_name == "DenoiserReflectionResolve")
	{
		return SlotID::DenoiserReflectionResolve;
	}
	if(slot_name == "DenoiserShadow_Prepare")
	{
		return SlotID::DenoiserShadow_Prepare;
	}
	if(slot_name == "DenoiserShadow_TileClassification")
	{
		return SlotID::DenoiserShadow_TileClassification;
	}
	if(slot_name == "DenoiserShadow_Filter")
	{
		return SlotID::DenoiserShadow_Filter;
	}
	if(slot_name == "DenoiserShadow_FilterLocal")
	{
		return SlotID::DenoiserShadow_FilterLocal;
	}
	if(slot_name == "DenoiserShadow_FilterLast")
	{
		return SlotID::DenoiserShadow_FilterLast;
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
	if(slot_name == "MeshInstanceInfo")
	{
		return SlotID::MeshInstanceInfo;
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
	if(slot_name == "GBuffer")
	{
		return SlotID::GBuffer;
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
	if(slot_name == "ReflectionCombine")
	{
		return SlotID::ReflectionCombine;
	}
	return std::nullopt;
}
uint get_table_index(SlotID id)
{
	if(id == SlotID::TextureRenderer)
	{
		return Slots::TextureRenderer::Slot::ID;
	}
	if(id == SlotID::BlueNoise)
	{
		return Slots::BlueNoise::Slot::ID;
	}
	if(id == SlotID::BRDF)
	{
		return Slots::BRDF::Slot::ID;
	}
	if(id == SlotID::DebugInfo)
	{
		return Slots::DebugInfo::Slot::ID;
	}
	if(id == SlotID::DenoiserReflectionCommon)
	{
		return Slots::DenoiserReflectionCommon::Slot::ID;
	}
	if(id == SlotID::DenoiserReflectionReproject)
	{
		return Slots::DenoiserReflectionReproject::Slot::ID;
	}
	if(id == SlotID::DenoiserReflectionPrefilter)
	{
		return Slots::DenoiserReflectionPrefilter::Slot::ID;
	}
	if(id == SlotID::DenoiserReflectionResolve)
	{
		return Slots::DenoiserReflectionResolve::Slot::ID;
	}
	if(id == SlotID::DenoiserShadow_Prepare)
	{
		return Slots::DenoiserShadow_Prepare::Slot::ID;
	}
	if(id == SlotID::DenoiserShadow_TileClassification)
	{
		return Slots::DenoiserShadow_TileClassification::Slot::ID;
	}
	if(id == SlotID::DenoiserShadow_Filter)
	{
		return Slots::DenoiserShadow_Filter::Slot::ID;
	}
	if(id == SlotID::DenoiserShadow_FilterLocal)
	{
		return Slots::DenoiserShadow_FilterLocal::Slot::ID;
	}
	if(id == SlotID::DenoiserShadow_FilterLast)
	{
		return Slots::DenoiserShadow_FilterLast::Slot::ID;
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
	if(id == SlotID::MeshInstanceInfo)
	{
		return Slots::MeshInstanceInfo::Slot::ID;
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
	if(id == SlotID::GBuffer)
	{
		return Slots::GBuffer::Slot::ID;
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
	if(id == SlotID::ReflectionCombine)
	{
		return Slots::ReflectionCombine::Slot::ID;
	}
	return -1;
}
std::string get_slot_name(SlotID id)
{
	if(id == SlotID::TextureRenderer)
	{
		return "TextureRenderer";
	}
	if(id == SlotID::BlueNoise)
	{
		return "BlueNoise";
	}
	if(id == SlotID::BRDF)
	{
		return "BRDF";
	}
	if(id == SlotID::DebugInfo)
	{
		return "DebugInfo";
	}
	if(id == SlotID::DenoiserReflectionCommon)
	{
		return "DenoiserReflectionCommon";
	}
	if(id == SlotID::DenoiserReflectionReproject)
	{
		return "DenoiserReflectionReproject";
	}
	if(id == SlotID::DenoiserReflectionPrefilter)
	{
		return "DenoiserReflectionPrefilter";
	}
	if(id == SlotID::DenoiserReflectionResolve)
	{
		return "DenoiserReflectionResolve";
	}
	if(id == SlotID::DenoiserShadow_Prepare)
	{
		return "DenoiserShadow_Prepare";
	}
	if(id == SlotID::DenoiserShadow_TileClassification)
	{
		return "DenoiserShadow_TileClassification";
	}
	if(id == SlotID::DenoiserShadow_Filter)
	{
		return "DenoiserShadow_Filter";
	}
	if(id == SlotID::DenoiserShadow_FilterLocal)
	{
		return "DenoiserShadow_FilterLocal";
	}
	if(id == SlotID::DenoiserShadow_FilterLast)
	{
		return "DenoiserShadow_FilterLast";
	}
	if(id == SlotID::FontRendering)
	{
		return "FontRendering";
	}
	if(id == SlotID::FontRenderingConstants)
	{
		return "FontRenderingConstants";
	}
	if(id == SlotID::FontRenderingGlyphs)
	{
		return "FontRenderingGlyphs";
	}
	if(id == SlotID::FrameInfo)
	{
		return "FrameInfo";
	}
	if(id == SlotID::FSR)
	{
		return "FSR";
	}
	if(id == SlotID::MaterialInfo)
	{
		return "MaterialInfo";
	}
	if(id == SlotID::MeshInstanceInfo)
	{
		return "MeshInstanceInfo";
	}
	if(id == SlotID::MeshInfo)
	{
		return "MeshInfo";
	}
	if(id == SlotID::GatherPipelineGlobal)
	{
		return "GatherPipelineGlobal";
	}
	if(id == SlotID::GatherPipeline)
	{
		return "GatherPipeline";
	}
	if(id == SlotID::GatherBoxes)
	{
		return "GatherBoxes";
	}
	if(id == SlotID::DrawBoxes)
	{
		return "DrawBoxes";
	}
	if(id == SlotID::InitDispatch)
	{
		return "InitDispatch";
	}
	if(id == SlotID::GatherMeshesBoxes)
	{
		return "GatherMeshesBoxes";
	}
	if(id == SlotID::MipMapping)
	{
		return "MipMapping";
	}
	if(id == SlotID::CopyTexture)
	{
		return "CopyTexture";
	}
	if(id == SlotID::DownsampleDepth)
	{
		return "DownsampleDepth";
	}
	if(id == SlotID::GBufferDownsample)
	{
		return "GBufferDownsample";
	}
	if(id == SlotID::GBufferQuality)
	{
		return "GBufferQuality";
	}
	if(id == SlotID::PSSMConstants)
	{
		return "PSSMConstants";
	}
	if(id == SlotID::PSSMData)
	{
		return "PSSMData";
	}
	if(id == SlotID::PSSMDataGlobal)
	{
		return "PSSMDataGlobal";
	}
	if(id == SlotID::PSSMLighting)
	{
		return "PSSMLighting";
	}
	if(id == SlotID::RaytracingRays)
	{
		return "RaytracingRays";
	}
	if(id == SlotID::Raytracing)
	{
		return "Raytracing";
	}
	if(id == SlotID::SceneData)
	{
		return "SceneData";
	}
	if(id == SlotID::GBuffer)
	{
		return "GBuffer";
	}
	if(id == SlotID::SkyData)
	{
		return "SkyData";
	}
	if(id == SlotID::SkyFace)
	{
		return "SkyFace";
	}
	if(id == SlotID::EnvFilter)
	{
		return "EnvFilter";
	}
	if(id == SlotID::EnvSource)
	{
		return "EnvSource";
	}
	if(id == SlotID::SMAA_Global)
	{
		return "SMAA_Global";
	}
	if(id == SlotID::SMAA_Weights)
	{
		return "SMAA_Weights";
	}
	if(id == SlotID::SMAA_Blend)
	{
		return "SMAA_Blend";
	}
	if(id == SlotID::Countour)
	{
		return "Countour";
	}
	if(id == SlotID::DrawStencil)
	{
		return "DrawStencil";
	}
	if(id == SlotID::PickerBuffer)
	{
		return "PickerBuffer";
	}
	if(id == SlotID::Instance)
	{
		return "Instance";
	}
	if(id == SlotID::Color)
	{
		return "Color";
	}
	if(id == SlotID::Test)
	{
		return "Test";
	}
	if(id == SlotID::NinePatch)
	{
		return "NinePatch";
	}
	if(id == SlotID::ColorRect)
	{
		return "ColorRect";
	}
	if(id == SlotID::FlowGraph)
	{
		return "FlowGraph";
	}
	if(id == SlotID::LineRender)
	{
		return "LineRender";
	}
	if(id == SlotID::VoxelInfo)
	{
		return "VoxelInfo";
	}
	if(id == SlotID::Voxelization)
	{
		return "Voxelization";
	}
	if(id == SlotID::VoxelScreen)
	{
		return "VoxelScreen";
	}
	if(id == SlotID::VoxelOutput)
	{
		return "VoxelOutput";
	}
	if(id == SlotID::VoxelBlur)
	{
		return "VoxelBlur";
	}
	if(id == SlotID::VoxelUpscale)
	{
		return "VoxelUpscale";
	}
	if(id == SlotID::VoxelVisibility)
	{
		return "VoxelVisibility";
	}
	if(id == SlotID::VoxelMipMap)
	{
		return "VoxelMipMap";
	}
	if(id == SlotID::VoxelCopy)
	{
		return "VoxelCopy";
	}
	if(id == SlotID::VoxelZero)
	{
		return "VoxelZero";
	}
	if(id == SlotID::VoxelLighting)
	{
		return "VoxelLighting";
	}
	if(id == SlotID::VoxelDebug)
	{
		return "VoxelDebug";
	}
	if(id == SlotID::DenoiserDownsample)
	{
		return "DenoiserDownsample";
	}
	if(id == SlotID::DenoiserHistoryFix)
	{
		return "DenoiserHistoryFix";
	}
	if(id == SlotID::TilingPostprocess)
	{
		return "TilingPostprocess";
	}
	if(id == SlotID::FrameClassification)
	{
		return "FrameClassification";
	}
	if(id == SlotID::FrameClassificationInitDispatch)
	{
		return "FrameClassificationInitDispatch";
	}
	if(id == SlotID::ReflectionCombine)
	{
		return "ReflectionCombine";
	}
	return "Unknown";
}
