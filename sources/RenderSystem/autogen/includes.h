#pragma once
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
#include "slots\MaterialInfo.h"
#include "tables\mesh_vertex_input.h"
#include "tables\AABB.h"
#include "tables\node_data.h"
#include "slots\MeshInfo.h"
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
#include "slots\VoxelInfo.h"
#include "slots\Voxelization.h"
#include "slots\VoxelScreen.h"
#include "slots\VoxelBlur.h"
#include "slots\VoxelUpscale.h"
#include "slots\VoxelMipMap.h"
#include "slots\VoxelLighting.h"
#include "slots\VoxelDebug.h"
#include "rt\GBuffer.h"
#include "pso\BRDF.h"
#include "pso\GatherPipeline.h"
#include "pso\GatherBoxes.h"
#include "pso\InitDispatch.h"
#include "pso\GatherMeshes.h"
#include "pso\DownsampleDepth.h"
#include "pso\MipMapping.h"
#include "pso\Lighting.h"
#include "pso\VoxelDownsample.h"
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
#include "pso\VoxelIndirectFilter.h"
#include "pso\VoxelReflectionHi.h"
#include "pso\VoxelReflectionUpsample.h"
#include "pso\VoxelIndirectHi.h"
#include "pso\VoxelIndirectLow.h"
#include "pso\VoxelIndirectUpsample.h"
#include "pso\VoxelDebug.h"
void init_signatures();
Render::RootLayout::ptr get_Signature(Layouts id);
void init_pso(enum_array<PSO, PSOBase::ptr>&);
