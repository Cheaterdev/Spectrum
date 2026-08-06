// Default/global-registration body for PSOS::MaterialPreview3D (see
// EnginePSOHolder::init -> autogen pso.cpp, which unconditionally builds one
// instance of every GraphicsPSO at startup). Never actually dispatched: the
// real per-material preview shader is built with fully inline source text
// (uniforms + material_preview_3d.hlsl + the generated node graph body) by
// MaterialPreviewSession::rebuild_pso, the same way the compute variant
// bypasses material_preview.hlsl's own default for the real
// UniversalMaterialPreview.hlsl. This just has to compile.
//
// MaterialPreviewInfo.h's table declares "MaterialCB data" as an opaque type
// name -- real per-material builds prepend the actual generated struct (see
// MaterialContext::generate_uniform_struct) before material_preview_3d.hlsl's
// text. This default has no material, so it needs its own placeholder,
// exactly like material_preview.hlsl's does for the compute PSO.
struct MaterialCB { int dummy; };

#include "autogen/MaterialPreviewInfo.h"

void PS_PREVIEW()
{
}
