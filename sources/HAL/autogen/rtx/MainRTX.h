#pragma once
#include "Shadow.h"
#include "Reflection.h"
#include "Indirect.h"
#include "ShadowPass.h"
#include "ColorPass.h"
struct MainRTX: public RTXPSO<MainRTX, Typelist<ShadowPass, ColorPass>, Typelist<Shadow, Reflection, Indirect>>
{
	static const constexpr Layouts global_sig  = Layouts::DefaultLayout;
	static const constexpr uint MaxTraceRecursionDepth = 2;
};
