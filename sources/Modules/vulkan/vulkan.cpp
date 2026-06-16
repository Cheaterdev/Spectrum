module;
// Global module fragment — provide the VMA implementation translation unit.
// VMA is a single-header library; exactly one TU must define VMA_IMPLEMENTATION
// before including the header.  The header unit in vulkan.ixx provides the
// declarations; this TU provides the definitions.
#pragma warning(push, 0)
#define VMA_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#pragma warning(pop)
module vulkan;
