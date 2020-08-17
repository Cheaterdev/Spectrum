#include "Common.hlsl"
#include "Common2D.hlsl"
#ifdef BUILD_FUNC_PS
#include "autogen/PickerBuffer.h"
#include "autogen/Instance.h"

static const RWStructuredBuffer<uint> viewBuffer = GetPickerBuffer().GetViewBuffer();


[earlydepthstencil]
void PS(vertex_output i)
{
	viewBuffer[0] = GetInstance().GetInstanceId();
}
#endif


float4 PS_RESULT(vertex_output i):SV_TARGET0
{
return 1;
}

#ifdef BUILD_FUNC_PS_COLOR
#include "autogen/Color.h"

float4 PS_COLOR(vertex_output i) :SV_TARGET0
{
return GetColor().GetColor();
}
#endif