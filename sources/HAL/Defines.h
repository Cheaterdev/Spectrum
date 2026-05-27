#pragma once
// =============================================================================
// HAL/Defines.h  —  HAL LAYER
// Macro entry-point for the HAL subproject and everything that depends on it.
// Chains upward to Core/Defines.h, then pulls in each canonical HAL macro
// header by #include so there is exactly one definition site per macro group.
// =============================================================================
#include "Core/Defines.h"

// ---- D3D12 result checking --------------------------------------------------
#include "D3D12/macros.h"

// ---- Render-target format lists (X-macro style) ----------------------------
#include "Formats.h"

// ---- PSO class body-injection macros ----------------------------------------
#include "SIG/PSO_defines.h"
