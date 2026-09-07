// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the shim pattern). Constants: generic zero locals (item 8's
// job to wire real values). Resources: real .sig struct.
#include "../autogen/SIGMA_SmoothTilesResources.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )   static const constantType constantName = (constantType)WaveGetLaneIndex();
#define NRD_CONSTANTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END \
	static const SamplerState gNearestClamp = pointClampSampler; \
	static const SamplerState gLinearClamp = linearClampSampler;

#define NRD_INPUTS_START   static const ConstantBuffer<SIGMA_SmoothTilesResources> _nrd_res = GetSIGMA_SmoothTilesResources();
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float3> gIn_Tiles = _nrd_res.GetGIn_Tiles();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float2> gOut_Tiles = _nrd_res.GetGOut_Tiles();

#include "SIGMA_SmoothTiles.cs.hlsl"
