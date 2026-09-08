#ifndef REBLUR_PACK_HELPER_HLSLI
#define REBLUR_PACK_HELPER_HLSLI

#include "3rdparty/NRD.hlsli"

// Shared front-end packing for NRD REBLUR_DIFFUSE's IN_DIFF_RADIANCE_HITDIST
// input (see [[project-nrd-integration]]): YCoCg radiance + normalized hit
// distance, per NRD.hlsli's own doc comments. Used by every raw indirect-GI
// signal that can feed REBLUR (IndirectRTX's raygen, and VoxelScreen's raw
// pre-history-lerp output) so they encode identically.
//
// gHitDistParams mirrors nrd::ReblurSettings::hitDistanceParameters' library
// default (A=3, B=0.1, C=20, see NRDSettings.h) -- both sides are left at
// NRD's defaults deliberately (see HAL.NRD.cpp), so this hardcoded copy must
// move together with that C++ default if either is ever tuned. Roughness is
// a constant 1.0: NRD's own diffuse-lobe call sites always pass 1.0 here
// (REBLUR_HitDistReconstruction.cs.hlsl et al).
float4 PackForReblurDiffuse(float3 radiance, float hitDist, float viewZ)
{
	const float3 gHitDistParams = float3(3.0, 0.1, 20.0);
	float normHitDist = REBLUR_FrontEnd_GetNormHitDist(hitDist, viewZ, gHitDistParams, 1.0);
	return REBLUR_FrontEnd_PackRadianceAndNormHitDist(radiance, normHitDist, true);
}

// Specular sibling of PackForReblurDiffuse, for NRD REBLUR_SPECULAR's
// IN_SPEC_RADIANCE_HITDIST input (see [[project-nrd-integration]]). Same
// gHitDistParams caveat as above. Unlike diffuse (which always passes a
// fixed roughness=1.0), specular's hit-distance normalization is lobe-width-
// dependent, so the real per-pixel roughness must be passed in.
float4 PackForReblurSpecular(float3 radiance, float hitDist, float roughness, float viewZ)
{
	const float3 gHitDistParams = float3(3.0, 0.1, 20.0);
	float normHitDist = REBLUR_FrontEnd_GetNormHitDist(hitDist, viewZ, gHitDistParams, roughness);
	return REBLUR_FrontEnd_PackRadianceAndNormHitDist(radiance, normHitDist, true);
}

#endif
