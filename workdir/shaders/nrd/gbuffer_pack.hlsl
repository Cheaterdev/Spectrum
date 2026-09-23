// Front-end packing for NRD REBLUR_DIFFUSE/REBLUR_SPECULAR (see
// [[project-nrd-integration]] and nrd_sig_test.prism's NRD_GBufferPack
// comment). Derives IN_VIEWZ (linear view-space Z), IN_NORMAL_ROUGHNESS
// (NRD_FrontEnd_PackNormalAndRoughness encoding) and IN_MV from the existing
// GBuffer, and packs IN_DIFF/SPEC_RADIANCE_HITDIST from whichever raw
// candidate (RTX or VCT) indirect_use_vct/reflection_use_vct selects -- see
// this pass's .prism comment for why packing lives here now instead of at
// each raygen, and why only the selected candidate, not both.
#include "../autogen/NRD_GBufferPackParams.h"
#include "../autogen/FrameInfo.h"
#include "../common/common.hlsl"
#include "3rdparty/NRD.hlsli"
#include "../nrd/reblur_pack_helper.hlsli"

static const Texture2D<float>    GBuffer_Depth       = GetNRD_GBufferPackParams().GetGbuffer().GetDepth();
static const Texture2D<float4>   GBuffer_Normals     = GetNRD_GBufferPackParams().GetGbuffer().GetNormals();
static const Texture2D<float2>   GBuffer_Speed       = GetNRD_GBufferPackParams().GetGbuffer().GetMotion();
static const Texture2D<float4>   In_RTXIndirect      = GetNRD_GBufferPackParams().GetRTXIndirectNoise();
static const Texture2D<float4>   In_RTXReflection    = GetNRD_GBufferPackParams().GetRTXReflectionNoise();
static const Texture2D<float4>   In_VCTIndirect      = GetNRD_GBufferPackParams().GetVoxelIndirectNoiseRaw();
static const Texture2D<float4>   In_VCTReflection    = GetNRD_GBufferPackParams().GetVoxelReflectionNoiseRaw();
static const RWTexture2D<float>  Out_ViewZ           = GetNRD_GBufferPackParams().GetNRD_ViewZ();
static const RWTexture2D<float4> Out_NormalRoughness = GetNRD_GBufferPackParams().GetNRD_NormalRoughness();
static const RWTexture2D<float4> Out_Mv              = GetNRD_GBufferPackParams().GetNRD_Mv();
static const RWTexture2D<float4> Out_Diffuse         = GetNRD_GBufferPackParams().GetNRD_DiffuseRadianceHitDist();
static const RWTexture2D<float4> Out_Specular        = GetNRD_GBufferPackParams().GetNRD_SpecularRadianceHitDist();

[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 size;
	Out_ViewZ.GetDimensions(size.x, size.y);
	if (any(dispatchID.xy >= size)) return;

	float raw_z = GBuffer_Depth[dispatchID.xy];

	// Sky/background (no geometry, same convention as
	// MyRaygenShaderIndirectRTXOnly's own raw_z==0 check): report a viewZ
	// past gDenoisingRange so REBLUR treats these pixels as out of range,
	// per NRD.hlsli's own recommendation ("use viewZ > denoisingRange for
	// INF (sky) pixels").
	float viewZ;
	if (raw_z == 0)
	{
		viewZ = 1e7;
	}
	else
	{
		const FrameInfo frame = CreateFrameInfo();
		float2 tc = (float2(dispatchID.xy) + 0.5) / size;
		float3 pos = depth_to_wpos(raw_z, tc, frame.GetCamera().GetInvViewProj());
		viewZ = mul(frame.GetCamera().GetView(), float4(pos, 1)).z;
	}
    Out_ViewZ[dispatchID.xy] = viewZ;

	// compress_normals() (FrameData.prism) always scales the unit normal by a
	// positive scalar before biasing to [0,1] -- re-normalizing recovers the
	// original direction exactly (same decode as normal_roughness_repack.hlsl).
	float4 encoded = GBuffer_Normals[dispatchID.xy];
	float3 normal = normalize(encoded.xyz * 2.0 - 1.0);
	float roughness = encoded.w;

	// materialID: no material-ID buffer exists in this engine yet, so this
	// is always 0 -- REBLUR_DIFFUSE only uses it for the optional strand/
	// camera-attached-reflection special cases (gStrandMaterialID etc,
	// REBLURSharedConstants), which this integration doesn't set up.
	Out_NormalRoughness[dispatchID.xy] = NRD_FrontEnd_PackNormalAndRoughness(normal, roughness, 0);

	// REBLUR's gIn_Mv is hardcoded Texture2D<float3> regardless of 2D/3D
	// mode (see this pass's .prism comment) -- GBuffer_Speed is only 2D
	// screen-space, so z is a constant 0 pad, matching
	// CommonSettings::motionVectorScale.z = 0 (set in HAL.NRD.cpp).
	Out_Mv[dispatchID.xy] = float4(GBuffer_Speed[dispatchID.xy], 0, 0);

	// Front-end pack only the selected raw (RGB=hit color, A=hit distance)
	// candidate per channel -- see this file's own top comment.
	bool  useVctIndirect   = GetNRD_GBufferPackParams().GetIndirect_use_vct()   != 0;
	bool  useVctReflection = GetNRD_GBufferPackParams().GetReflection_use_vct() != 0;
	float4 indirectRaw   = useVctIndirect   ? In_VCTIndirect[dispatchID.xy]   : In_RTXIndirect[dispatchID.xy];
	float4 reflectionRaw = useVctReflection ? In_VCTReflection[dispatchID.xy] : In_RTXReflection[dispatchID.xy];

	Out_Diffuse[dispatchID.xy]  = PackForReblurDiffuse(indirectRaw.rgb, indirectRaw.a, viewZ);
	Out_Specular[dispatchID.xy] = PackForReblurSpecular(reflectionRaw.rgb, reflectionRaw.a, roughness, viewZ);
}
