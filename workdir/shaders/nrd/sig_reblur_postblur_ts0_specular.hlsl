// REBLUR_SPECULAR sibling of sig_reblur_postblur_ts0.hlsl (see
// [[project-nrd-integration]]). TEMPORAL_STABILIZATION=0: PostBlur also
// emits gOut_InternalData/gOut_SpecCopy since the TemporalStabilization pass
// is disabled and won't produce them itself. Routed via
// REBLUR_PostBlurTS0SpecularResources (Diff->Spec renamed, same resource
// count -- see nrd_sig_test.prism's comment on that struct).
#define NRD_INTERNAL
#define NRD_SIGNAL SPEC
#define NRD_MODE RADIANCE

#include "../autogen/REBLUR_PostBlurTS0SpecularResources.h"

#define NRD_CONSTANTS_START( resourceName ) \
	static const ConstantBuffer<REBLUR_PostBlurTS0SpecularResources> _nrd_res = GetREBLUR_PostBlurTS0SpecularResources(); \
	static const REBLURSharedConstants _nrd_shared = _nrd_res.GetSharedConstants();
#define NRD_CONSTANT( constantType, constantName )
#define NRD_CONSTANTS_END \
	static const float4x4 gWorldToClip = _nrd_shared.GetGWorldToClip(); \
	static const float4x4 gViewToClip = _nrd_shared.GetGViewToClip(); \
	static const float4x4 gViewToWorld = _nrd_shared.GetGViewToWorld(); \
	static const float4x4 gWorldToViewPrev = _nrd_shared.GetGWorldToViewPrev(); \
	static const float4x4 gWorldToClipPrev = _nrd_shared.GetGWorldToClipPrev(); \
	static const float4x4 gWorldPrevToWorld = _nrd_shared.GetGWorldPrevToWorld(); \
	static const float4 gRotatorPre = _nrd_shared.GetGRotatorPre(); \
	static const float4 gRotator = _nrd_shared.GetGRotator(); \
	static const float4 gRotatorPost = _nrd_shared.GetGRotatorPost(); \
	static const float4 gFrustum = _nrd_shared.GetGFrustum(); \
	static const float4 gFrustumPrev = _nrd_shared.GetGFrustumPrev(); \
	static const float4 gCameraDelta = _nrd_shared.GetGCameraDelta(); \
	static const float4 gHitDistSettings = _nrd_shared.GetGHitDistSettings(); \
	static const float4 gViewVectorWorld = _nrd_shared.GetGViewVectorWorld(); \
	static const float4 gViewVectorWorldPrev = _nrd_shared.GetGViewVectorWorldPrev(); \
	static const float4 gMvScale = _nrd_shared.GetGMvScale(); \
	static const float4 gConvergenceSettings = _nrd_shared.GetGConvergenceSettings(); \
	static const float2 gAntilagSettings = _nrd_shared.GetGAntilagSettings(); \
	static const float2 gResourceSize = _nrd_shared.GetGResourceSize(); \
	static const float2 gResourceSizeInv = _nrd_shared.GetGResourceSizeInv(); \
	static const float2 gResourceSizeInvPrev = _nrd_shared.GetGResourceSizeInvPrev(); \
	static const float2 gRectSize = _nrd_shared.GetGRectSize(); \
	static const float2 gRectSizeInv = _nrd_shared.GetGRectSizeInv(); \
	static const float2 gRectSizePrev = _nrd_shared.GetGRectSizePrev(); \
	static const float2 gResolutionScale = _nrd_shared.GetGResolutionScale(); \
	static const float2 gResolutionScalePrev = _nrd_shared.GetGResolutionScalePrev(); \
	static const float2 gRectOffset = _nrd_shared.GetGRectOffset(); \
	static const float2 gJitter = _nrd_shared.GetGJitter(); \
	static const uint2 gPrintfAt = _nrd_shared.GetGPrintfAt(); \
	static const uint2 gRectOrigin = _nrd_shared.GetGRectOrigin(); \
	static const int2 gRectSizeMinusOne = _nrd_shared.GetGRectSizeMinusOne(); \
	static const float gDisocclusionThreshold = _nrd_shared.GetGDisocclusionThreshold(); \
	static const float gDisocclusionThresholdAlternate = _nrd_shared.GetGDisocclusionThresholdAlternate(); \
	static const float gCameraAttachedReflectionMaterialID = _nrd_shared.GetGCameraAttachedReflectionMaterialID(); \
	static const float gStrandMaterialID = _nrd_shared.GetGStrandMaterialID(); \
	static const float gStrandThickness = _nrd_shared.GetGStrandThickness(); \
	static const float gStabilizationStrength = _nrd_shared.GetGStabilizationStrength(); \
	static const float gDebug = _nrd_shared.GetGDebug(); \
	static const float gOrthoMode = _nrd_shared.GetGOrthoMode(); \
	static const float gUnproject = _nrd_shared.GetGUnproject(); \
	static const float gDenoisingRange = _nrd_shared.GetGDenoisingRange(); \
	static const float gPlaneDistSensitivity = _nrd_shared.GetGPlaneDistSensitivity(); \
	static const float gFramerateScale = _nrd_shared.GetGFramerateScale(); \
	static const float gMinBlurRadius = _nrd_shared.GetGMinBlurRadius(); \
	static const float gMaxBlurRadius = _nrd_shared.GetGMaxBlurRadius(); \
	static const float gDiffPrepassBlurRadius = _nrd_shared.GetGDiffPrepassBlurRadius(); \
	static const float gSpecPrepassBlurRadius = _nrd_shared.GetGSpecPrepassBlurRadius(); \
	static const float gMaxAccumulatedFrameNum = _nrd_shared.GetGMaxAccumulatedFrameNum(); \
	static const float gMaxFastAccumulatedFrameNum = _nrd_shared.GetGMaxFastAccumulatedFrameNum(); \
	static const float gAntiFirefly = _nrd_shared.GetGAntiFirefly(); \
	static const float gLobeAngleFraction = _nrd_shared.GetGLobeAngleFraction(); \
	static const float gRoughnessFraction = _nrd_shared.GetGRoughnessFraction(); \
	static const float gHistoryFixFrameNum = _nrd_shared.GetGHistoryFixFrameNum(); \
	static const float gHistoryFixBasePixelStride = _nrd_shared.GetGHistoryFixBasePixelStride(); \
	static const float gHistoryFixAlternatePixelStride = _nrd_shared.GetGHistoryFixAlternatePixelStride(); \
	static const float gHistoryFixAlternatePixelStrideMaterialID = _nrd_shared.GetGHistoryFixAlternatePixelStrideMaterialID(); \
	static const float gFastHistoryClampingSigmaScale = _nrd_shared.GetGFastHistoryClampingSigmaScale(); \
	static const float gMinRectDimMulUnproject = _nrd_shared.GetGMinRectDimMulUnproject(); \
	static const float gUsePrepassNotOnlyForSpecularMotionEstimation = _nrd_shared.GetGUsePrepassNotOnlyForSpecularMotionEstimation(); \
	static const float gSplitScreen = _nrd_shared.GetGSplitScreen(); \
	static const float gSplitScreenPrev = _nrd_shared.GetGSplitScreenPrev(); \
	static const float gCheckerboardResolveAccumSpeed = _nrd_shared.GetGCheckerboardResolveAccumSpeed(); \
	static const float gViewZScale = _nrd_shared.GetGViewZScale(); \
	static const float gFireflySuppressorMinRelativeScale = _nrd_shared.GetGFireflySuppressorMinRelativeScale(); \
	static const float gMinHitDistanceWeight = _nrd_shared.GetGMinHitDistanceWeight(); \
	static const float gDiffMinMaterial = _nrd_shared.GetGDiffMinMaterial(); \
	static const float gSpecMinMaterial = _nrd_shared.GetGSpecMinMaterial(); \
	static const float gResponsiveAccumulationInvRoughnessThreshold = _nrd_shared.GetGResponsiveAccumulationInvRoughnessThreshold(); \
	static const uint gResponsiveAccumulationMinAccumulatedFrameNum = _nrd_shared.GetGResponsiveAccumulationMinAccumulatedFrameNum(); \
	static const uint gHasHistoryConfidence = _nrd_shared.GetGHasHistoryConfidence(); \
	static const uint gHasDisocclusionThresholdMix = _nrd_shared.GetGHasDisocclusionThresholdMix(); \
	static const uint gDiffCheckerboard = _nrd_shared.GetGDiffCheckerboard(); \
	static const uint gSpecCheckerboard = _nrd_shared.GetGSpecCheckerboard(); \
	static const uint gFrameIndex = _nrd_shared.GetGFrameIndex(); \
	static const uint gIsRectChanged = _nrd_shared.GetGIsRectChanged(); \
	static const uint gResetHistory = _nrd_shared.GetGResetHistory(); \
	static const uint gReturnHistoryLengthInsteadOfOcclusion = _nrd_shared.GetGReturnHistoryLengthInsteadOfOcclusion();

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END \
	static const SamplerState gNearestClamp = pointClampSampler; \
	static const SamplerState gLinearClamp = linearClampSampler;

#define NRD_INPUTS_START
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float> gIn_Tiles = _nrd_res.GetGIn_Tiles(); \
	static const Texture2D<float4> gIn_Normal_Roughness = _nrd_res.GetGIn_Normal_Roughness(); \
	static const Texture2D<float2> gIn_Data1 = _nrd_res.GetGIn_Data1(); \
	static const Texture2D<float> gIn_ViewZ = _nrd_res.GetGIn_ViewZ(); \
	static const Texture2D<float4> gIn_Spec = _nrd_res.GetGIn_Spec();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float4> gOut_Normal_Roughness = _nrd_res.GetGOut_Normal_Roughness(); \
	static const RWTexture2D<float4> gOut_Spec = _nrd_res.GetGOut_Spec(); \
	static const RWTexture2D<uint> gOut_InternalData = _nrd_res.GetGOut_InternalData(); \
	static const RWTexture2D<float4> gOut_SpecCopy = _nrd_res.GetGOut_SpecCopy();

#define TEMPORAL_STABILIZATION 0

#include "3rdparty/REBLUR_PostBlur.cs.hlsl"
