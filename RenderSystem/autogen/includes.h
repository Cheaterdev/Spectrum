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
#include "slots\MeshData.h"
#include "slots\MeshNodes.h"
#include "slots\MipMapping.h"
#include "slots\CopyTexture.h"
#include "slots\DownsampleDepth.h"
#include "slots\PSSMConstants.h"
#include "slots\PSSMData.h"
#include "slots\PSSMLighting.h"
#include "slots\SkyData.h"
#include "slots\SkyFace.h"
#include "slots\Test.h"
#include "slots\NinePatch.h"
#include "slots\ColorRect.h"
#include "slots\FlowGraph.h"
enum class Layouts: int
{
	FrameLayout,
	DefaultLayout,
	TOTAL
};
void init_signatures();
Render::RootSignature::ptr get_Signature(Layouts id);
