// Item 7 SIG port (see [[project-nrd-integration]] and sig_clear.hlsl's
// comment for the shim pattern). No NRD_SAMPLERS_START block in
// SIGMA_Copy.resources.hlsli itself, but all 12 macros must still be
// #defined for NRD.hlsli's "custom engine" branch detection to activate.
#include "../autogen/SIGMA_CopyResources.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )   static const constantType constantName = (constantType)WaveGetLaneIndex();
#define NRD_CONSTANTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END

#define NRD_INPUTS_START   static const ConstantBuffer<SIGMA_CopyResources> _nrd_res = GetSIGMA_CopyResources();
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END \
	static const Texture2D<float2> gIn_Tiles = _nrd_res.GetGIn_Tiles(); \
	static const Texture2D<float4> gIn_History = _nrd_res.GetGIn_History(); \
	static const Texture2D<uint> gIn_HistoryLength = _nrd_res.GetGIn_HistoryLength();

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float4> gOut_History = _nrd_res.GetGOut_History(); \
	static const RWTexture2D<uint> gOut_HistoryLength = _nrd_res.GetGOut_HistoryLength();

#include "3rdparty/SIGMA_Copy.cs.hlsl"
