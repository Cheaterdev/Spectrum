export module HAL:Autogen.RTX.MainRTX;

import Core;
import :RTX;
import :Enums;
import :Types;

import :Autogen.Tables.ShadowPayload;
import :Autogen.Tables.RayPayload;
import :Autogen.Slots.MaterialInfo;
export {
#include "Shadow.h"
#include "Reflection.h"
#include "Indirect.h"
#include "ShadowPass.h"
#include "ColorPass.h"

struct MainRTX: public RTXPSO<MainRTX, Typelist<ShadowPass, ColorPass>, Typelist<Shadow, Reflection, Indirect>>
{
	static const constexpr Layouts global_sig = Layouts::DefaultLayout;
	static const constexpr uint MaxTraceRecursionDepth = 2;
};
}
