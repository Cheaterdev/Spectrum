#pragma once
#include "layout\FrameLayout.h"
#include "layout\DefaultLayout.h"
#include "slots\TextureRenderer.h"
#include "slots\BRDF.h"
#include "slots\FontRendering.h"
#include "slots\FontRenderingConstants.h"
#include "slots\FrameInfo.h"
#include "slots\MaterialInfo.h"
#include "slots\SceneData.h"
#include "slots\MeshInfo.h"
#include "slots\GatherPipelineGlobal.h"
#include "slots\GatherPipeline.h"
#include "slots\MipMapping.h"
#include "slots\CopyTexture.h"
#include "slots\DownsampleDepth.h"
#include "slots\PSSMConstants.h"
#include "slots\PSSMData.h"
#include "slots\PSSMLighting.h"
#include "slots\SkyData.h"
#include "slots\SkyFace.h"
#include "slots\Countour.h"
#include "slots\PickerBuffer.h"
#include "slots\Instance.h"
#include "slots\Color.h"
#include "slots\Test.h"
#include "slots\NinePatch.h"
#include "slots\ColorRect.h"
#include "slots\FlowGraph.h"
void init_signatures();
Render::RootLayout::ptr get_Signature(Layouts id);
