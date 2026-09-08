// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the shim pattern).
#include "../autogen/SIGMA_SplitScreenResources.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )   static const constantType constantName = (constantType)WaveGetLaneIndex();
#define NRD_CONSTANTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END \
	static const SamplerState gNearestClamp = pointClampSampler; \
	static const SamplerState gLinearClamp = linearClampSampler;

#define NRD_INPUTS_START   static const ConstantBuffer<SIGMA_SplitScreenResources> _nrd_res = GetSIGMA_SplitScreenResources();
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float> gIn_ViewZ = _nrd_res.GetGIn_ViewZ(); \
	static const Texture2D<float> gIn_Penumbra = _nrd_res.GetGIn_Penumbra();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float> gOut_Shadow_Translucency = _nrd_res.GetGOut_Shadow_Translucency();

#define TRANSLUCENCY 0

#include "3rdparty/SIGMA_SplitScreen.cs.hlsl"
