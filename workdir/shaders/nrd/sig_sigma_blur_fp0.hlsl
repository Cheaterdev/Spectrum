// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the shim pattern). FIRST_PASS=0 permutation: gIn_Shadow_Translucency
// IS compiled in (FIRST_PASS==0 makes the #if(FIRST_PASS==0||TRANSLUCENCY==1) true).
#include "../autogen/SIGMA_BlurFirstPass0Resources.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )   static const constantType constantName = (constantType)WaveGetLaneIndex();
#define NRD_CONSTANTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END \
	static const SamplerState gNearestClamp = pointClampSampler; \
	static const SamplerState gLinearClamp = linearClampSampler;

#define NRD_INPUTS_START   static const ConstantBuffer<SIGMA_BlurFirstPass0Resources> _nrd_res = GetSIGMA_BlurFirstPass0Resources();
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float> gIn_ViewZ = _nrd_res.GetGIn_ViewZ(); \
	static const Texture2D<float4> gIn_Normal_Roughness = _nrd_res.GetGIn_Normal_Roughness(); \
	static const Texture2D<float> gIn_Penumbra = _nrd_res.GetGIn_Penumbra(); \
	static const Texture2D<float2> gIn_Tiles = _nrd_res.GetGIn_Tiles(); \
	static const Texture2D<float> gIn_Shadow_Translucency = _nrd_res.GetGIn_Shadow_Translucency();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float> gOut_Penumbra = _nrd_res.GetGOut_Penumbra(); \
	static const RWTexture2D<float> gOut_Shadow_Translucency = _nrd_res.GetGOut_Shadow_Translucency();

#define TRANSLUCENCY 0
#define FIRST_PASS 0

#include "SIGMA_Blur.cs.hlsl"
