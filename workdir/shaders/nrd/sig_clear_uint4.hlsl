// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the shim pattern). Clear.cs.hlsl|FLOAT=0 permutation -- gOut is
// RWTexture2D<uint4> instead of <float4>, hence a separate struct/PSO from
// sig_clear.hlsl's Clear_Constants/NRD_Clear_Test (FLOAT=1).
#include "../autogen/Clear_UInt4Resources.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )   static const constantType constantName = (constantType)WaveGetLaneIndex();
#define NRD_CONSTANTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END

#define NRD_INPUTS_START
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END

#define NRD_OUTPUTS_START   static const ConstantBuffer<Clear_UInt4Resources> _nrd_res = GetClear_UInt4Resources();
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<uint4> gOut = _nrd_res.GetGOut();

#define FLOAT 0

#include "Clear.cs.hlsl"
