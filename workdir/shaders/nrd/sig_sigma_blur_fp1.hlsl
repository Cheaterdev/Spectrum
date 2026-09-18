// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the shim pattern). FIRST_PASS=1 permutation: no
// gIn_Shadow_Translucency (only compiled when FIRST_PASS==0||TRANSLUCENCY==1).
// Constants route through the real SIGMASharedConstants CBV (see
// HAL.NRD.cpp's fill_sigma_shared_constants).
#include "../autogen/SIGMA_BlurFirstPass1Resources.h"

#define NRD_CONSTANTS_START( resourceName ) \
	static const ConstantBuffer<SIGMA_BlurFirstPass1Resources> _nrd_res = GetSIGMA_BlurFirstPass1Resources(); \
	static const SIGMASharedConstants _nrd_shared = _nrd_res.GetSharedConstants();
#define NRD_CONSTANT( constantType, constantName )
#define NRD_CONSTANTS_END \
	static const float4x4 gWorldToView = _nrd_shared.GetGWorldToView(); \
	static const float4x4 gViewToClip = _nrd_shared.GetGViewToClip(); \
	static const float4x4 gWorldToClipPrev = _nrd_shared.GetGWorldToClipPrev(); \
	static const float4x4 gWorldToViewPrev = _nrd_shared.GetGWorldToViewPrev(); \
	static const float4 gRotator = _nrd_shared.GetGRotator(); \
	static const float4 gRotatorPost = _nrd_shared.GetGRotatorPost(); \
	static const float4 gViewVectorWorld = _nrd_shared.GetGViewVectorWorld(); \
	static const float4 gLightDirectionView = _nrd_shared.GetGLightDirectionView(); \
	static const float4 gFrustum = _nrd_shared.GetGFrustum(); \
	static const float4 gFrustumPrev = _nrd_shared.GetGFrustumPrev(); \
	static const float4 gCameraDelta = _nrd_shared.GetGCameraDelta(); \
	static const float4 gMvScale = _nrd_shared.GetGMvScale(); \
	static const float2 gResourceSizeInv = _nrd_shared.GetGResourceSizeInv(); \
	static const float2 gResourceSizeInvPrev = _nrd_shared.GetGResourceSizeInvPrev(); \
	static const float2 gRectSize = _nrd_shared.GetGRectSize(); \
	static const float2 gRectSizeInv = _nrd_shared.GetGRectSizeInv(); \
	static const float2 gRectSizePrev = _nrd_shared.GetGRectSizePrev(); \
	static const float2 gResolutionScale = _nrd_shared.GetGResolutionScale(); \
	static const float2 gRectOffset = _nrd_shared.GetGRectOffset(); \
	static const uint2 gPrintfAt = _nrd_shared.GetGPrintfAt(); \
	static const uint2 gRectOrigin = _nrd_shared.GetGRectOrigin(); \
	static const int2 gRectSizeMinusOne = _nrd_shared.GetGRectSizeMinusOne(); \
	static const int2 gTilesSizeMinusOne = _nrd_shared.GetGTilesSizeMinusOne(); \
	static const float gOrthoMode = _nrd_shared.GetGOrthoMode(); \
	static const float gUnproject = _nrd_shared.GetGUnproject(); \
	static const float gDenoisingRange = _nrd_shared.GetGDenoisingRange(); \
	static const float gPlaneDistSensitivity = _nrd_shared.GetGPlaneDistSensitivity(); \
	static const float gStabilizationStrength = _nrd_shared.GetGStabilizationStrength(); \
	static const float gDebug = _nrd_shared.GetGDebug(); \
	static const float gSplitScreen = _nrd_shared.GetGSplitScreen(); \
	static const float gViewZScale = _nrd_shared.GetGViewZScale(); \
	static const float gMinRectDimMulUnproject = _nrd_shared.GetGMinRectDimMulUnproject(); \
	static const uint gFrameIndex = _nrd_shared.GetGFrameIndex(); \
	static const uint gIsRectChanged = _nrd_shared.GetGIsRectChanged();

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END \
	static const SamplerState gNearestClamp = pointClampSampler; \
	static const SamplerState gLinearClamp = linearClampSampler;

#define NRD_INPUTS_START
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float> gIn_ViewZ = _nrd_res.GetGIn_ViewZ(); \
	static const Texture2D<float4> gIn_Normal_Roughness = _nrd_res.GetGIn_Normal_Roughness(); \
	static const Texture2D<float> gIn_Penumbra = _nrd_res.GetGIn_Penumbra(); \
	static const Texture2D<float2> gIn_Tiles = _nrd_res.GetGIn_Tiles();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float> gOut_Penumbra = _nrd_res.GetGOut_Penumbra(); \
	static const RWTexture2D<float> gOut_Shadow_Translucency = _nrd_res.GetGOut_Shadow_Translucency();

#define TRANSLUCENCY 0
#define FIRST_PASS 1

#include "3rdparty/SIGMA_Blur.cs.hlsl"
