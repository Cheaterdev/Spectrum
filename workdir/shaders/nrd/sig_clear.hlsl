// Prototype (see [[project-nrd-integration]]): routes NRD's real,
// unmodified Clear.cs.hlsl through this engine's SIG-generated bindless
// accessors (sources/SIGParser/sigs/nrd_sig_test.sig's Clear_Constants
// struct) instead of NRD.hlsli's normal raw register()-bound globals.
//
// NRD.hlsli's own DXC branch is skipped entirely once NRD_CONSTANTS_START/
// NRD_CONSTANT/NRD_CONSTANTS_END/NRD_INPUTS_START/NRD_INPUT/NRD_INPUTS_END/
// NRD_OUTPUTS_START/NRD_OUTPUT/NRD_OUTPUTS_END/NRD_SAMPLERS_START/
// NRD_SAMPLER/NRD_SAMPLERS_END are ALL pre-defined -- see NRD.hlsli's own
// "Custom engine that defined all the macros" branch.
//
// Not a generic reusable shim: SIG capitalizes the first letter of each
// field's accessor (gDebug -> GetGDebug()), which a token-pasted
// Get##fieldName() macro can't reproduce (no case conversion in the
// preprocessor) -- so the four accessor calls below are hand-written to
// match this one struct's generated names, rather than derived generically
// from NRD_CONSTANT/NRD_OUTPUT's own arguments.
#include "../autogen/Clear_Constants.h"

#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )
#define NRD_CONSTANTS_END \
	static const ConstantBuffer<Clear_Constants> _nrd_cb = GetClear_Constants(); \
	static const float gDebug = _nrd_cb.GetGDebug(); \
	static const float gViewZScale = _nrd_cb.GetGViewZScale(); \
	static const float gDenoisingRange = _nrd_cb.GetGDenoisingRange();

#define NRD_INPUTS_START
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_INPUTS_END

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex )
#define NRD_OUTPUTS_END \
	static const RWTexture2D<float4> gOut = _nrd_cb.GetGOut();

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END

#define FLOAT 1

#include "3rdparty/Clear.cs.hlsl"
