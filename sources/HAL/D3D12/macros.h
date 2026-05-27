#pragma once
// D3D12 result-checking macro for HAL and all dependent projects.
// Included by HAL/Defines.h via forced include.
// STRINGIZE is inherited from Core/Utils/utils_macros.h — not redefined here.

#ifndef TEST
#  define TEST(device, x)  (device).process_result(x, STRINGIZE(x))
#endif
