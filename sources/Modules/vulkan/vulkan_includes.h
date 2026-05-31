#pragma once
// vulkan_includes.h — Vulkan API declarations for use as a C++20 header unit.
// Included by vulkan.ixx via  export import "vulkan_includes.h".
// Provides VkXxx types, vulkan_win32 surface extension, and VMA declarations.
// VMA_IMPLEMENTATION is defined in vulkan.cpp (global module fragment) so the
// implementation symbols are compiled exactly once.

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
// vcpkg installs VMA under the vma/ subdirectory.
#include <vma/vk_mem_alloc.h>

// ---- Macro re-exports ---------------------------------------------------
// C++20 named modules cannot export preprocessor macros, so any Vulkan macro
// used as a value in module code must be re-exported as a real C++ entity
// (same approach as Modules/d3d12/d3d12_includes.h for IID_PPV_ARGS / DXGI
// error codes).
//
// VK_NULL_HANDLE is `#define VK_NULL_HANDLE 0`.  We re-export it as `nullptr`
// rather than an integer constant: on x64 (VK_USE_64_BIT_PTR_DEFINES) every
// Vulkan handle is a pointer type, and only a null-pointer constant — not a
// constexpr int 0 — is assignable to a pointer.  All inline header code that
// needs the macro (e.g. VMA) was already parsed above with the macro active,
// so undefining it here is safe.
#undef VK_NULL_HANDLE
export inline constexpr decltype(nullptr) VK_NULL_HANDLE = nullptr;
