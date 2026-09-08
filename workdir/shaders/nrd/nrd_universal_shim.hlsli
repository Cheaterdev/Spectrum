// Spike (see [[project-nrd-integration]]): the ONE generic resource shim
// every NRD kernel wrapper includes now, replacing per-kernel hand-typed
// NRD_INPUT/NRD_OUTPUT redirections. NRD's own *.resources.hlsli files pass
// resourceType/dataType/resourceName/bindingIndex as literal macro
// arguments already -- bindingIndex is the correct per-category (t.../u...)
// index, so this needs no counters or per-kernel field lists: index straight
// into NRD_Universal's slotsIn/slotsOut (see nrd_sig_test.sig's comment),
// cast via ResourceDescriptorHeap the same way slot.jinja's own
// ConstantBuffer<T> accessors already do.
//
// Calls GetNRD_Universal() directly at every use instead of caching a local
// -- not a NRD_INPUTS_START-time local, since some kernels (Clear) never
// call NRD_INPUTS_START/END at all (zero inputs), only NRD_OUTPUTS_START/
// END. GetNRD_Universal() itself just returns the file-scope
// nrd_universal_global slot.jinja already generates (computed once via
// ResourceDescriptorHeap[pass_NRD_Universal.offset]), so calling it more
// than once costs nothing extra.
// NRD_Universal.h's own guard (SLOT_6) protects against two DIFFERENT
// structs both claiming that slot, not against being #include'd twice in
// the same compile -- it has no #pragma once. A kernel that also pulls in
// reblur_shared_constants_shim.hlsli (which needs the same header for
// pass_NRD_Universal) would otherwise trip that guard as a false "slot
// already used" error. This guard is the real include-once protection,
// shared by name with reblur_shared_constants_shim.hlsli so whichever shim
// is included first wins and the second is a no-op.
#ifndef NRD_UNIVERSAL_INCLUDED
#define NRD_UNIVERSAL_INCLUDED
#include "../autogen/NRD_Universal.h"
#endif

#define NRD_INPUTS_START
#define NRD_INPUT( resourceType, dataType, resourceName, regName, bindingIndex ) \
	static const resourceType<dataType> resourceName = ResourceDescriptorHeap[GetNRD_Universal().GetSlotsIn(bindingIndex)];
#define NRD_INPUTS_END

#define NRD_OUTPUTS_START
#define NRD_OUTPUT( resourceType, dataType, resourceName, regName, bindingIndex ) \
	static const resourceType<dataType> resourceName = ResourceDescriptorHeap[GetNRD_Universal().GetSlotsOut(bindingIndex)];
#define NRD_OUTPUTS_END

#define NRD_SAMPLERS_START
#define NRD_SAMPLER( resourceType, resourceName, regName, bindingIndex )
#define NRD_SAMPLERS_END
