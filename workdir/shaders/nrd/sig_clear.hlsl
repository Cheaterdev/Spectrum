// Spike (see [[project-nrd-integration]]): first kernel moved onto the
// universal NRD_Universal struct/shim (nrd_universal_shim.hlsli) instead of
// its own hand-typed Clear_Constants struct -- proves the shared shim
// compiles and dispatches correctly before the rest of the kernels move.
//
// Clear.resources.hlsli calls NRD_CONSTANTS_START/NRD_CONSTANT/END for
// gDebug/gViewZScale/gDenoisingRange, "only for availability in
// Common.hlsl" per its own comment -- and that's not just about Clear.cs.hlsl's
// own body (which indeed never reads them): Common.hlsli #include's a bunch
// of OTHER helper functions unrelated to Clear (e.g. ApplyGeometryWeightLast)
// that reference gDenoisingRange, and DXC type-checks every function body in
// a compiled file eagerly regardless of whether Clear's own entry point ever
// calls it -- leaving these undeclared breaks the whole file, not just
// Clear's own (never-reached) uses. Real values never matter here (NRD's own
// reflection drops this pipeline's constant blob entirely -- see
// HAL.NRD.cpp's generalized-from-dispatch_clear assert on
// constantBufferDataSize), so plain zero locals are enough to satisfy every
// OTHER function's compile, same effect the original per-kernel
// Clear_Constants struct had by construction (memcpy of 0 bytes leaves its
// fields zero-initialized).
#define NRD_CONSTANTS_START( resourceName )
#define NRD_CONSTANT( constantType, constantName )
#define NRD_CONSTANTS_END \
	static const float gDebug = 0; \
	static const float gViewZScale = 0; \
	static const float gDenoisingRange = 0;

#include "nrd_universal_shim.hlsli"

#define FLOAT 1

#include "3rdparty/Clear.cs.hlsl"
