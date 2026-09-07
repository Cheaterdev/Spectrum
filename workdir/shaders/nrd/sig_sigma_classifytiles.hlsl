// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the general shim pattern). Constants are generic
// zero-initialized locals here -- wiring their real per-dispatch values is
// item 8, not this pass's job; only the Texture2D/RWTexture2D resource
// fields route through the real .sig struct (SIGMA_ClassifyTilesResources),
// since those need a real, valid, type-matched descriptor to avoid
// GPU-visible undefined behavior even before item 8 lands.
#include "../autogen/SIGMA_ClassifyTilesResources.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )   static const constantType constantName = (constantType)WaveGetLaneIndex();
#define NRD_CONSTANTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END \
	static const SamplerState gNearestClamp = pointClampSampler; \
	static const SamplerState gLinearClamp = linearClampSampler;

#define NRD_INPUTS_START   static const ConstantBuffer<SIGMA_ClassifyTilesResources> _nrd_res = GetSIGMA_ClassifyTilesResources();
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float> gIn_ViewZ = _nrd_res.GetGIn_ViewZ(); \
	static const Texture2D<float> gIn_Penumbra = _nrd_res.GetGIn_Penumbra();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float4> gOut_Tiles = _nrd_res.GetGOut_Tiles();

// Matches the actual compiled permutation (SIGMA_SHADOW is non-translucent).
#define TRANSLUCENCY 0

#include "SIGMA_ClassifyTiles.cs.hlsl"
