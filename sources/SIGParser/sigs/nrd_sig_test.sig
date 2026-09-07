# Prototype only (see [[project-nrd-integration]]): can NRD's own vendored
# shader source compile against this engine's SIG/bindless reflection instead
# of a hand-built root signature? Uses Clear.cs.hlsl (workdir/shaders/nrd/) --
# the smallest NRD kernel: 3 constants, 1 RWTexture2D output, no inputs, no
# samplers -- via a macro shim (workdir/shaders/nrd/sig_clear.hlsl) that
# redirects NRD.hlsli's NRD_CONSTANTS_START/NRD_CONSTANT/NRD_OUTPUTS_START/
# NRD_OUTPUT macros at this struct's generated accessors instead of raw
# register()s. Struct name MUST be "Clear_Constants" -- it has to match
# NRD_CONSTANTS_START( Clear_Constants ) in Clear.resources.hlsli exactly,
# since the shim macro token-pastes that name into Get##name().
#
# Reinstated for debugging (see [[project-nrd-integration]]): removing this
# PSO correlated with a wrap->clamp sampler bug on mesh rendering/material
# previews going away, but no causal mechanism was found -- NRD_Clear_Test's
# samplers are baked as isolated D3D12_STATIC_SAMPLER_DESC entries in its own
# root signature (no shared heap), and DefaultLayout's generated files were
# byte-identical with/without it. Brought back at user's request to debug
# directly rather than treat the correlation as proof.
[Bind = DefaultLayout::Instance2]
struct Clear_Constants
{
	float gDebug;
	float gViewZScale;
	float gDenoisingRange;
	RWTexture2D<float4> gOut;
}

ComputePSO NRD_Clear_Test
{
	root = DefaultLayout;

	[EntryPoint = main]
	compute = nrd/sig_clear;
}

# Item 7 (see [[project-nrd-integration]]): the rest of NRD's SIGMA_SHADOW +
# REBLUR_DIFFUSE kernel set, ported to SIG the same way as Clear_Constants
# above -- one .sig struct + ComputePSO + shim per kernel/permutation. Scoped
# to compiling + PSO creation only (item 7), not correct dispatch (item 8):
# each struct declares ONLY the Texture2D/RWTexture2D resource fields (the
# things that genuinely need a real, valid, type-matched descriptor slot to
# avoid GPU-visible undefined behavior); the many NRD_CONSTANT-declared
# scalar/matrix constants (SIGMA_SHARED_CONSTANTS/REBLUR_SHARED_CONSTANTS,
# 32/65 fields respectively) are left as generic zero-initialized locals by
# each shim rather than routed through a .sig CBV -- wiring their real values
# from GetComputeDispatches()'s constantBufferData is item 8's job, kernel by
# kernel, not this pass's.
#
# Resource lists below are for this instance's ACTUAL compiled permutation
# only (NRD_SIGNAL=DIFF, NRD_MODE=RADIANCE, TRANSLUCENCY=0 -- REBLUR_DIFFUSE
# is diffuse-only, SIGMA_SHADOW is the non-translucent variant), read
# directly off each kernel's .resources.hlsli #if branches -- not the union
# of every possible permutation.
[Bind = DefaultLayout::Instance2]
struct SIGMA_ClassifyTilesResources
{
	Texture2D<float> gIn_ViewZ;
	Texture2D<float> gIn_Penumbra;
	RWTexture2D<float4> gOut_Tiles;
}

ComputePSO NRD_SIGMA_ClassifyTiles
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_classifytiles;
}

[Bind = DefaultLayout::Instance2]
struct SIGMA_SmoothTilesResources
{
	Texture2D<float3> gIn_Tiles;
	RWTexture2D<float2> gOut_Tiles;
}

ComputePSO NRD_SIGMA_SmoothTiles
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_smoothtiles;
}

[Bind = DefaultLayout::Instance2]
struct SIGMA_CopyResources
{
	Texture2D<float2> gIn_Tiles;
	Texture2D<float4> gIn_History;
	Texture2D<uint> gIn_HistoryLength;
	RWTexture2D<float4> gOut_History;
	RWTexture2D<uint> gOut_HistoryLength;
}

ComputePSO NRD_SIGMA_Copy
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_copy;
}

# FIRST_PASS=1: no gIn_Shadow_Translucency (only compiled in when
# FIRST_PASS==0 || TRANSLUCENCY==1, and TRANSLUCENCY=0 here).
[Bind = DefaultLayout::Instance2]
struct SIGMA_BlurFirstPass1Resources
{
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_Penumbra;
	Texture2D<float2> gIn_Tiles;
	RWTexture2D<float> gOut_Penumbra;
	RWTexture2D<float> gOut_Shadow_Translucency;
}

ComputePSO NRD_SIGMA_BlurFirstPass1
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_blur_fp1;
}

# FIRST_PASS=0: gIn_Shadow_Translucency IS compiled in.
[Bind = DefaultLayout::Instance2]
struct SIGMA_BlurFirstPass0Resources
{
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_Penumbra;
	Texture2D<float2> gIn_Tiles;
	Texture2D<float> gIn_Shadow_Translucency;
	RWTexture2D<float> gOut_Penumbra;
	RWTexture2D<float> gOut_Shadow_Translucency;
}

ComputePSO NRD_SIGMA_BlurFirstPass0
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_blur_fp0;
}

[Bind = DefaultLayout::Instance2]
struct SIGMA_TemporalStabilizationResources
{
	Texture2D<float> gIn_ViewZ;
	Texture2D<float3> gIn_Mv;
	Texture2D<float> gIn_Penumbra;
	Texture2D<float> gIn_Shadow_Translucency;
	Texture2D<float> gIn_History;
	Texture2D<uint> gIn_HistoryLength;
	Texture2D<float2> gIn_Tiles;
	RWTexture2D<float> gOut_Shadow_Translucency;
	RWTexture2D<uint> gOut_HistoryLength;
}

ComputePSO NRD_SIGMA_TemporalStabilization
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_temporalstabilization;
}

[Bind = DefaultLayout::Instance2]
struct SIGMA_SplitScreenResources
{
	Texture2D<float> gIn_ViewZ;
	Texture2D<float> gIn_Penumbra;
	RWTexture2D<float> gOut_Shadow_Translucency;
}

ComputePSO NRD_SIGMA_SplitScreen
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_sigma_splitscreen;
}

# Item 8 (see [[project-nrd-integration]]): REBLUR_SHARED_CONSTANTS
# (REBLUR_Config.hlsli:115-192), verbatim field-for-field/order-for-order --
# 77 NRD_CONSTANT(type,name) entries, reused identically by 9 of REBLUR's 10
# kernels (Validation embeds this plus 2 extra uint fields of its own, see
# REBLUR_ValidationResources below). Embedded (not [Bind] on its own) into
# each REBLUR_*Resources struct below, same plain-struct-composition pattern
# as Frustum/Camera in FrameData.sig -- accessed as
# resources.GetSharedConstants().GetGWorldToClip() etc from C++. Field name
# is "sharedConstants", not "shared" -- "shared" is an HLSL storage-class
# keyword (groupshared-adjacent), and DXC misparses "REBLURSharedConstants
# shared;" as a modifier instead of an identifier.
struct REBLURSharedConstants
{
	float4x4 gWorldToClip;
	float4x4 gViewToClip;
	float4x4 gViewToWorld;
	float4x4 gWorldToViewPrev;
	float4x4 gWorldToClipPrev;
	float4x4 gWorldPrevToWorld;
	float4 gRotatorPre;
	float4 gRotator;
	float4 gRotatorPost;
	float4 gFrustum;
	float4 gFrustumPrev;
	float4 gCameraDelta;
	float4 gHitDistSettings;
	float4 gViewVectorWorld;
	float4 gViewVectorWorldPrev;
	float4 gMvScale;
	float4 gConvergenceSettings;
	float2 gAntilagSettings;
	float2 gResourceSize;
	float2 gResourceSizeInv;
	float2 gResourceSizeInvPrev;
	float2 gRectSize;
	float2 gRectSizeInv;
	float2 gRectSizePrev;
	float2 gResolutionScale;
	float2 gResolutionScalePrev;
	float2 gRectOffset;
	float2 gJitter;
	uint2 gPrintfAt;
	uint2 gRectOrigin;
	int2 gRectSizeMinusOne;
	float gDisocclusionThreshold;
	float gDisocclusionThresholdAlternate;
	float gCameraAttachedReflectionMaterialID;
	float gStrandMaterialID;
	float gStrandThickness;
	float gStabilizationStrength;
	float gDebug;
	float gOrthoMode;
	float gUnproject;
	float gDenoisingRange;
	float gPlaneDistSensitivity;
	float gFramerateScale;
	float gMinBlurRadius;
	float gMaxBlurRadius;
	float gDiffPrepassBlurRadius;
	float gSpecPrepassBlurRadius;
	float gMaxAccumulatedFrameNum;
	float gMaxFastAccumulatedFrameNum;
	float gAntiFirefly;
	float gLobeAngleFraction;
	float gRoughnessFraction;
	float gHistoryFixFrameNum;
	float gHistoryFixBasePixelStride;
	float gHistoryFixAlternatePixelStride;
	float gHistoryFixAlternatePixelStrideMaterialID;
	float gFastHistoryClampingSigmaScale;
	float gMinRectDimMulUnproject;
	float gUsePrepassNotOnlyForSpecularMotionEstimation;
	float gSplitScreen;
	float gSplitScreenPrev;
	float gCheckerboardResolveAccumSpeed;
	float gViewZScale;
	float gFireflySuppressorMinRelativeScale;
	float gMinHitDistanceWeight;
	float gDiffMinMaterial;
	float gSpecMinMaterial;
	float gResponsiveAccumulationInvRoughnessThreshold;
	uint gResponsiveAccumulationMinAccumulatedFrameNum;
	uint gHasHistoryConfidence;
	uint gHasDisocclusionThresholdMix;
	uint gDiffCheckerboard;
	uint gSpecCheckerboard;
	uint gFrameIndex;
	uint gIsRectChanged;
	uint gResetHistory;
	uint gReturnHistoryLengthInsteadOfOcclusion;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_ClassifyTilesResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_ViewZ;
	RWTexture2D<float> gOut_Tiles;
}

ComputePSO NRD_REBLUR_ClassifyTiles
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_classifytiles;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_HitDistReconstructionResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Diff;
	RWTexture2D<float4> gOut_Diff;
}

ComputePSO NRD_REBLUR_HitDistReconstruction
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_hitdistreconstruction;
}

# MODE_5X5=1 permutation -- same resource layout as MODE_5X5=0 (the #if only
# affects NRD_BORDER, not resources.hlsli's declarations), but NRD reports it
# as a distinct pipeline, so it needs its own PSO for item 8's dispatch loop.
ComputePSO NRD_REBLUR_HitDistReconstruction5x5
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_hitdistreconstruction_5x5;
}

# REBLUR_SPECULAR's permutation of the same kernel (see
# [[project-nrd-integration]] and HAL.NRD.cpp's dispatch.identifier routing) --
# same resource *count* as diffuse (4 in + 1 out), Diff->Spec renamed, per
# REBLUR_HitDistReconstruction.resources.hlsli's NRD_SPEC-only branch.
[Bind = DefaultLayout::Instance2]
struct REBLUR_HitDistReconstructionSpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Spec;
	RWTexture2D<float4> gOut_Spec;
}

ComputePSO NRD_REBLUR_HitDistReconstruction_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_hitdistreconstruction_specular;
}

ComputePSO NRD_REBLUR_HitDistReconstruction5x5_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_hitdistreconstruction_5x5_specular;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_PrePassResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Diff;
	RWTexture2D<float4> gOut_Diff;
}

ComputePSO NRD_REBLUR_PrePass
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_prepass;
}

# REBLUR_SPECULAR's permutation -- one extra output (gOut_SpecHitDistForTracking,
# no diffuse equivalent) vs the diffuse struct, per
# REBLUR_PrePass.resources.hlsli's NRD_SPEC-only branch.
[Bind = DefaultLayout::Instance2]
struct REBLUR_PrePassSpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Spec;
	RWTexture2D<float4> gOut_Spec;
	RWTexture2D<float> gOut_SpecHitDistForTracking;
}

ComputePSO NRD_REBLUR_PrePass_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_prepass_specular;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_TemporalAccumulationResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float3> gIn_Mv;
	Texture2D<float> gPrev_ViewZ;
	Texture2D<float4> gPrev_Normal_Roughness;
	Texture2D<uint> gPrev_InternalData;
	Texture2D<float> gIn_DisocclusionThresholdMix;
	Texture2D<float> gIn_DiffConfidence;
	Texture2D<float4> gIn_Diff;
	Texture2D<float4> gHistory_Diff;
	Texture2D<float> gHistory_DiffFast;
	RWTexture2D<float2> gOut_Data1;
	RWTexture2D<float4> gOut_Diff;
	RWTexture2D<float> gOut_DiffFast;
	RWTexture2D<uint> gOut_Data2;
}

ComputePSO NRD_REBLUR_TemporalAccumulation
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_temporalaccumulation;
}

# REBLUR_SPECULAR's permutation -- per
# REBLUR_TemporalAccumulation.resources.hlsli's NRD_SPEC-only branch: same
# shared prefix (0-7), then gIn_SpecConfidence/gIn_Spec/gHistory_Spec/
# gHistory_SpecFast/gPrev_SpecHitDistForTracking/gIn_SpecHitDistForTracking
# (14 inputs total, vs diffuse's 12 -- specular tracks an extra hit-distance
# pair diffuse doesn't need), and gOut_Data1/gOut_Spec/gOut_SpecFast/
# gOut_SpecHitDistForTracking/gOut_Data2 (5 outputs, vs diffuse's 4).
[Bind = DefaultLayout::Instance2]
struct REBLUR_TemporalAccumulationSpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float3> gIn_Mv;
	Texture2D<float> gPrev_ViewZ;
	Texture2D<float4> gPrev_Normal_Roughness;
	Texture2D<uint> gPrev_InternalData;
	Texture2D<float> gIn_DisocclusionThresholdMix;
	Texture2D<float> gIn_SpecConfidence;
	Texture2D<float4> gIn_Spec;
	Texture2D<float4> gHistory_Spec;
	Texture2D<float> gHistory_SpecFast;
	Texture2D<float> gPrev_SpecHitDistForTracking;
	Texture2D<float> gIn_SpecHitDistForTracking;
	RWTexture2D<float2> gOut_Data1;
	RWTexture2D<float4> gOut_Spec;
	RWTexture2D<float> gOut_SpecFast;
	RWTexture2D<float> gOut_SpecHitDistForTracking;
	RWTexture2D<uint> gOut_Data2;
}

ComputePSO NRD_REBLUR_TemporalAccumulation_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_temporalaccumulation_specular;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_HistoryFixResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float2> gIn_Data1;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Diff;
	Texture2D<float> gIn_DiffFast;
	RWTexture2D<float4> gOut_Diff;
	RWTexture2D<float> gOut_DiffFast;
}

ComputePSO NRD_REBLUR_HistoryFix
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_historyfix;
}

# REBLUR_SPECULAR's permutation -- one extra input (gIn_SpecHitDistForTracking,
# no diffuse equivalent) vs the diffuse struct, per
# REBLUR_HistoryFix.resources.hlsli's NRD_SPEC-only branch.
[Bind = DefaultLayout::Instance2]
struct REBLUR_HistoryFixSpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float2> gIn_Data1;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Spec;
	Texture2D<float> gIn_SpecFast;
	Texture2D<float> gIn_SpecHitDistForTracking;
	RWTexture2D<float4> gOut_Spec;
	RWTexture2D<float> gOut_SpecFast;
}

ComputePSO NRD_REBLUR_HistoryFix_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_historyfix_specular;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_BlurResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float2> gIn_Data1;
	Texture2D<float4> gIn_Diff;
	RWTexture2D<float> gOut_ViewZ;
	RWTexture2D<float4> gOut_Diff;
}

ComputePSO NRD_REBLUR_Blur
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_blur;
}

# REBLUR_SPECULAR's permutation -- same resource count as diffuse (5 in + 2
# out), Diff->Spec renamed, per REBLUR_Blur.resources.hlsli's NRD_SPEC-only
# branch.
[Bind = DefaultLayout::Instance2]
struct REBLUR_BlurSpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float2> gIn_Data1;
	Texture2D<float4> gIn_Spec;
	RWTexture2D<float> gOut_ViewZ;
	RWTexture2D<float4> gOut_Spec;
}

ComputePSO NRD_REBLUR_Blur_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_blur_specular;
}

# TEMPORAL_STABILIZATION=0: PostBlur also emits gOut_InternalData/gOut_DiffCopy
# (TemporalStabilization pass is disabled, so PostBlur itself must produce
# the data those would have carried forward).
[Bind = DefaultLayout::Instance2]
struct REBLUR_PostBlurTS0Resources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float2> gIn_Data1;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Diff;
	RWTexture2D<float4> gOut_Normal_Roughness;
	RWTexture2D<float4> gOut_Diff;
	RWTexture2D<uint> gOut_InternalData;
	RWTexture2D<float4> gOut_DiffCopy;
}

ComputePSO NRD_REBLUR_PostBlurTS0
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_postblur_ts0;
}

# REBLUR_SPECULAR's permutation -- same resource count as diffuse (5 in + 4
# out), Diff->Spec renamed, per REBLUR_PostBlur.resources.hlsli's
# TEMPORAL_STABILIZATION=0, NRD_SPEC-only branch.
[Bind = DefaultLayout::Instance2]
struct REBLUR_PostBlurTS0SpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float2> gIn_Data1;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Spec;
	RWTexture2D<float4> gOut_Normal_Roughness;
	RWTexture2D<float4> gOut_Spec;
	RWTexture2D<uint> gOut_InternalData;
	RWTexture2D<float4> gOut_SpecCopy;
}

ComputePSO NRD_REBLUR_PostBlurTS0_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_postblur_ts0_specular;
}

# TEMPORAL_STABILIZATION=1: the TemporalStabilization pass runs afterward and
# owns gOut_InternalData itself, so PostBlur skips producing it here.
[Bind = DefaultLayout::Instance2]
struct REBLUR_PostBlurTS1Resources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float2> gIn_Data1;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Diff;
	RWTexture2D<float4> gOut_Normal_Roughness;
	RWTexture2D<float4> gOut_Diff;
}

ComputePSO NRD_REBLUR_PostBlurTS1
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_postblur_ts1;
}

# REBLUR_SPECULAR's permutation -- same resource count as diffuse (5 in + 2
# out), Diff->Spec renamed, per REBLUR_PostBlur.resources.hlsli's
# TEMPORAL_STABILIZATION=1, NRD_SPEC-only branch.
[Bind = DefaultLayout::Instance2]
struct REBLUR_PostBlurTS1SpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float2> gIn_Data1;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Spec;
	RWTexture2D<float4> gOut_Normal_Roughness;
	RWTexture2D<float4> gOut_Spec;
}

ComputePSO NRD_REBLUR_PostBlurTS1_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_postblur_ts1_specular;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_TemporalStabilizationResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float2> gIn_Data1;
	Texture2D<uint> gIn_Data2;
	Texture2D<float4> gIn_Diff;
	Texture2D<float> gHistory_DiffLumaStabilized;
	RWTexture2D<float4> gInOut_Mv;
	RWTexture2D<uint> gOut_InternalData;
	RWTexture2D<float4> gOut_Diff;
	RWTexture2D<float> gOut_DiffLumaStabilized;
}

ComputePSO NRD_REBLUR_TemporalStabilization
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_temporalstabilization;
}

# REBLUR_SPECULAR's permutation -- per
# REBLUR_TemporalStabilization.resources.hlsli's NRD_SPEC-only branch: one
# extra input (gIn_SpecHitDistForTracking, no diffuse equivalent) vs the
# diffuse struct (8 inputs vs 7), same 4 outputs (Diff->Spec renamed).
[Bind = DefaultLayout::Instance2]
struct REBLUR_TemporalStabilizationSpecularResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_Tiles;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float2> gIn_Data1;
	Texture2D<uint> gIn_Data2;
	Texture2D<float> gIn_SpecHitDistForTracking;
	Texture2D<float4> gIn_Spec;
	Texture2D<float> gHistory_SpecLumaStabilized;
	RWTexture2D<float4> gInOut_Mv;
	RWTexture2D<uint> gOut_InternalData;
	RWTexture2D<float4> gOut_Spec;
	RWTexture2D<float> gOut_SpecLumaStabilized;
}

ComputePSO NRD_REBLUR_TemporalStabilization_Specular
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_temporalstabilization_specular;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_SplitScreenResources
{
	REBLURSharedConstants sharedConstants;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float4> gIn_Diff;
	RWTexture2D<float4> gOut_Diff;
}

ComputePSO NRD_REBLUR_SplitScreen
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_splitscreen;
}

[Bind = DefaultLayout::Instance2]
struct REBLUR_ValidationResources
{
	REBLURSharedConstants sharedConstants;
	uint gHasDiffuse;
	uint gHasSpecular;
	Texture2D<float4> gIn_Normal_Roughness;
	Texture2D<float> gIn_ViewZ;
	Texture2D<float3> gIn_Mv;
	Texture2D<float2> gIn_Data1;
	Texture2D<uint> gIn_Data2;
	Texture2D<float4> gIn_Diff;
	Texture2D<float4> gIn_Spec;
	RWTexture2D<float4> gOut_Validation;
}

ComputePSO NRD_REBLUR_Validation
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_reblur_validation;
}

# Clear.cs.hlsl|FLOAT=0 -- the uint4 permutation of the kernel already ported
# (FLOAT=1) as Clear_Constants/NRD_Clear_Test. Separate struct: gOut's type
# differs (uint4 vs float4), so it can't share Clear_Constants' name/shape.
[Bind = DefaultLayout::Instance2]
struct Clear_UInt4Resources
{
	RWTexture2D<uint4> gOut;
}

ComputePSO NRD_Clear_UInt4
{
	root = DefaultLayout;
	[EntryPoint = main]
	compute = nrd/sig_clear_uint4;
}

# Front-end packing for NRD REBLUR_DIFFUSE (see [[project-nrd-integration]]):
# derives IN_VIEWZ (linear view-space Z), IN_NORMAL_ROUGHNESS (NRD's own pack
# encoding, NRD_FrontEnd_PackNormalAndRoughness) and IN_MV from the engine's
# existing GBuffer in one combined dispatch. NRD_Mv exists (rather than
# binding GBuffer_Speed directly) because NRD's own REBLUR_TemporalAccumulation.
# resources.hlsli hardcodes gIn_Mv as Texture2D<float3> regardless of 2D/3D
# motion mode (the mode only changes what the .z component MEANS, via
# CommonSettings::motionVectorScale.z -- see REBLUR_TemporalAccumulation.cs.
# hlsl's "gIn_Mv[...] * gMvScale.xyz"), but GBuffer_Speed is a 2-channel
# R16G16_FLOAT -- z is repacked here as a constant 0 to match. Physical
# format is still RGBA16F (no 3-channel D3D12 format exists); NRD's kernels
# only ever read the first 3 components.
# IN_DIFF_RADIANCE_HITDIST is packed at the source in raytracing.hlsl's
# MyRaygenShaderIndirectRTXOnly, not here.
[Bind = DefaultLayout::Instance2]
struct NRD_GBufferPackParams
{
	GBuffer gbuffer;
	RWTexture2D<float> NRD_ViewZ;
	RWTexture2D<float4> NRD_NormalRoughness;
	RWTexture2D<float4> NRD_Mv;
}

ComputePSO NRD_GBufferPack
{
	root = DefaultLayout;
	[EntryPoint = CS]
	compute = nrd/gbuffer_pack;
}

[Static]
PassNode NRD_GBufferPack
{
	GBuffer gbuffer;

	[Write] Texture NRD_ViewZ;
	[Write] Texture NRD_NormalRoughness;
	[Write] Texture NRD_Mv;
}

# Debug-only: unpacks RTXIndirectDenoised's raw REBLUR encoding (YCoCg +
# normalized hit-distance, see raytracing.hlsl's packing comment) into plain
# RGB, purely so the debug-view dropdown ("RTX Indirect (REBLUR), Unpacked")
# can show a true-color preview -- the real consumer, RTXCombine, does its
# own unpack in rtx_combine.hlsl and doesn't use this texture.
[Bind = DefaultLayout::Instance2]
struct NRD_UnpackDebugParams
{
	Texture2D<float4> Packed;
	RWTexture2D<float4> Unpacked;
}

ComputePSO NRD_UnpackDebug
{
	root = DefaultLayout;
	[EntryPoint = CS]
	compute = nrd/unpack_debug;
}

# Real per-frame REBLUR_DIFFUSE execution (see [[project-nrd-integration]]).
# No ComputePSO of its own -- like UpscalingDLSSRR, this is an opaque
# multi-dispatch C++ call (nvidia::NRD::get().execute(), HAL.NRD.cpp) issuing
# NRD's own returned dispatch list against the 21 kernel PSOs already
# declared above, not a single shader this engine compiles directly.
[Static]
PassNode NRD_REBLUR_Execute
{
	Texture NRD_ViewZ;
	Texture NRD_NormalRoughness;
	Texture NRD_Mv;
	Texture RTXIndirectNoise;
	# VoxelScreen's raw (pre-history-lerp) signal -- alternative diff_noisy
	# input selected by g_indirect_source instead of RTXIndirectNoise.
	Texture VoxelIndirectNoiseRaw;
	Texture RTXReflectionNoise;
	# ScreenReflection's raw (pre-ReflectionDenoiser_Reproject) signal --
	# alternative spec_noisy input selected by g_reflection_source instead of
	# RTXReflectionNoise.
	Texture VoxelReflectionNoiseRaw;

	[Write] Texture RTXIndirectDenoised;
	[Write] Texture RTXIndirectDenoisedPreview;
	[Write] Texture RTXReflectionDenoised;
}

# The FSR/DLSS-side equivalent of the indirect term RTXCombine computes for
# DLSS-RR (see [[project-nrd-integration]]): adds REBLUR's denoised indirect
# GI onto ResultTexture, using the same shading formula VoxelCombine's own
# blur pass uses for its (undenoised) indirect term. Runs only when NRD is
# selected and DLSS-RR is NOT the active upscaler -- RTXCombine already
# covers DLSS-RR+NRD, and VoxelCombine's own composite (voxel_screen_blur.hlsl)
# defers to this pass via VoxelBlur::skip_composite in exactly this case.
[Bind = DefaultLayout::Instance0]
struct NRD_IndirectCombineParams
{
	GBuffer gbuffer;
	Texture2D<float4> indirect;

	RWTexture2D<float4> target;
}

ComputePSO NRD_IndirectCombine
{
	root = DefaultLayout;
	[EntryPoint = CS]
	compute = nrd/nrd_indirect_combine;
}

[Static]
[Compute]
PassNode NRD_IndirectCombine
{
	GBuffer gbuffer;
	Texture RTXIndirectDenoised;

	[Write] Texture ResultTexture;
}
