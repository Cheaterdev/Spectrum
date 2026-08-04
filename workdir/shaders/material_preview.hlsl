// Default/global-registration body for PSOS::MaterialPreview (see
// EnginePSOHolder::init -> autogen pso.cpp, which unconditionally builds one
// instance of every ComputePSO at startup). Never actually dispatched: the
// real per-material preview shader is built with fully inline source text
// (uniforms + UniversalMaterialPreview.hlsl + the generated node graph body)
// by materials::universal_material, the same way GBufferDraw's per-material
// pixel shader bypasses its own default file. This just has to compile.
//
// MaterialPreviewInfo.h's table declares "MaterialCB data" as an opaque type
// name -- real per-material builds prepend the actual generated struct (see
// MaterialContext::generate_uniform_struct) before this file's text. This
// default has no material, so it needs its own placeholder, exactly like
// generate_uniform_struct() falls back to "int dummy;" when there are no
// uniforms.
struct MaterialCB { int dummy; };

#include "autogen/MaterialPreviewInfo.h"

[numthreads(8, 8, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
}
