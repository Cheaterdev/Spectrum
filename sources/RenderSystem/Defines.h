#pragma once
// =============================================================================
// RenderSystem/Defines.h  —  RENDERSYSTEM LAYER
// Render-system macros.  Chains upward to HAL/Defines.h.
// =============================================================================
#include "HAL/Defines.h"

// ============================================================================
// Enum-string helper  (formerly RenderSystem/FrameGraph/defines.h)
// Usage:  enum class Foo { H(Bar), H(Baz) };
// Expands each enumerator to  Bar = "Bar"  so the enumerator name is its own
// string representation.
// ============================================================================
#define H(x)  x = #x

#define REGISTER_MATERIAL_NODE(TYPE) static MaterialNodeRegister<TYPE> _reg_##TYPE(#TYPE);