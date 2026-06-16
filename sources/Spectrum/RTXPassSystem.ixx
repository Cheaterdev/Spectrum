export module Graphics:RTXPassSystem;


import :FrameGraphContext;
import :Context;
import :MeshRenderer;
import FrameGraph;
import HAL;

#define A_CPU
#include "bend_sss_cpu.h"

#include "../RenderSystem/FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
