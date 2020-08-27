#pragma once
#include "layout\FrameLayout.h"
#include "layout\DefaultLayout.h"
#include "slots\TextureRenderer.h"
#include "slots\BRDF.h"
#include "slots\FontRendering.h"
#include "slots\FontRenderingConstants.h"
#include "slots\FrameInfo.h"
#include "slots\MaterialInfo.h"
#include "slots\MeshInfo.h"
#include "slots\GatherPipelineGlobal.h"
#include "slots\GatherPipeline.h"
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
#include "slots\Raytracing.h"
#include "slots\SceneData.h"
#include "slots\SkyData.h"
#include "slots\SkyFace.h"
#include "slots\EnvFilter.h"
#include "slots\EnvSource.h"
#include "slots\SkyData.h"
#include "slots\SkyFace.h"
#include "slots\EnvFilter.h"
#include "slots\EnvSource.h"
#include "slots\Countour.h"
#include "slots\DrawStencil.h"
#include "slots\PickerBuffer.h"
#include "slots\Instance.h"
#include "slots\Color.h"
#include "slots\Test.h"
#include "slots\NinePatch.h"
#include "slots\ColorRect.h"
#include "slots\FlowGraph.h"
#include "slots\VoxelInfo.h"
#include "slots\Voxelization.h"
#include "slots\VoxelScreen.h"
#include "slots\VoxelBlur.h"
#include "slots\VoxelUpscale.h"
#include "slots\VoxelMipMap.h"
#include "slots\VoxelLighting.h"
#include "slots\VoxelDebug.h"
#include "rt\GBuffer.h"
void init_signatures();
Render::RootLayout::ptr get_Signature(Layouts id);
