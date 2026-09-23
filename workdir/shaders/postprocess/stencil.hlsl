#include "../common/common.hlsl"
#include "../common/common_2d.hlsl"

#ifdef BUILD_FUNC_PS_COLOR
#include "../autogen/Color.h"
#include "gizmo_pick.hlsl"

float4 PS_COLOR(vertex_output i) :SV_TARGET0
{
	gizmo_pick(i.pos);
	return GetColor().GetColor();
}
#endif
