#pragma once
// X-macro lists of HAL render-target formats.
// Included by HAL/Defines.h via forced include.
// Usage: expand at call site as a comma-separated list of HAL::Format values.

#ifndef ALL_RT_FORMATS
#define ALL_RT_FORMATS \
    HAL::Format::R32G32B32A32_FLOAT,    \
    HAL::Format::R32G32B32A32_UINT,     \
    HAL::Format::R32G32B32A32_SINT,     \
    HAL::Format::R32G32B32_FLOAT,       \
    HAL::Format::R32G32B32_UINT,        \
    HAL::Format::R32G32B32_SINT,        \
    HAL::Format::R16G16B16A16_FLOAT,    \
    HAL::Format::R16G16B16A16_UNORM,    \
    HAL::Format::R16G16B16A16_UINT,     \
    HAL::Format::R16G16B16A16_SNORM,    \
    HAL::Format::R16G16B16A16_SINT,     \
    HAL::Format::R32G32_FLOAT,          \
    HAL::Format::R32G32_UINT,           \
    HAL::Format::R32G32_SINT,           \
    HAL::Format::R10G10B10A2_UNORM,     \
    HAL::Format::R10G10B10A2_UINT,      \
    HAL::Format::R11G11B10_FLOAT,       \
    HAL::Format::R8G8B8A8_UNORM,        \
    HAL::Format::R8G8B8A8_UNORM_SRGB,   \
    HAL::Format::R8G8B8A8_UINT,         \
    HAL::Format::R8G8B8A8_SNORM,        \
    HAL::Format::R8G8B8A8_SINT,         \
    HAL::Format::R16G16_FLOAT,          \
    HAL::Format::R16G16_UNORM,          \
    HAL::Format::R16G16_UINT,           \
    HAL::Format::R16G16_SNORM,          \
    HAL::Format::R16G16_SINT,           \
    HAL::Format::R32_FLOAT,             \
    HAL::Format::R32_UINT,              \
    HAL::Format::R32_SINT,              \
    HAL::Format::R8G8_UNORM,            \
    HAL::Format::R8G8_UINT,             \
    HAL::Format::R8G8_SNORM,            \
    HAL::Format::R8G8_SINT,             \
    HAL::Format::R16_FLOAT,             \
    HAL::Format::R16_UNORM,             \
    HAL::Format::R16_UINT,              \
    HAL::Format::R16_SNORM,             \
    HAL::Format::R16_SINT,              \
    HAL::Format::R8_UNORM,              \
    HAL::Format::R8_UINT,               \
    HAL::Format::R8_SNORM,              \
    HAL::Format::R8_SINT,               \
    HAL::Format::A8_UNORM,              \
    /*HAL::Format::R1_UNORM,*/          \
    HAL::Format::R8G8_B8G8_UNORM,       \
    HAL::Format::G8R8_G8B8_UNORM,       \
    HAL::Format::B5G6R5_UNORM,          \
    HAL::Format::B5G5R5A1_UNORM,        \
    HAL::Format::B8G8R8A8_UNORM,        \
    HAL::Format::B8G8R8X8_UNORM,        \
    HAL::Format::B8G8R8A8_UNORM_SRGB,   \
    HAL::Format::B8G8R8X8_UNORM_SRGB
#endif

#ifndef ALL_RT_BLENDING_FORMATS
#define ALL_RT_BLENDING_FORMATS \
    HAL::Format::R32G32B32A32_FLOAT,    \
    HAL::Format::R16G16B16A16_FLOAT,    \
    HAL::Format::R16G16B16A16_UNORM,    \
    HAL::Format::R16G16B16A16_SNORM,    \
    HAL::Format::R32G32_FLOAT,          \
    HAL::Format::R10G10B10A2_UNORM,     \
    HAL::Format::R11G11B10_FLOAT,       \
    HAL::Format::R8G8B8A8_UNORM,        \
    HAL::Format::R8G8B8A8_UNORM_SRGB,   \
    HAL::Format::R8G8B8A8_SNORM,        \
    HAL::Format::R16G16_FLOAT,          \
    HAL::Format::R16G16_UNORM,          \
    HAL::Format::R16G16_SNORM,          \
    HAL::Format::R32_FLOAT,             \
    HAL::Format::R8G8_UNORM,            \
    HAL::Format::R8G8_SNORM,            \
    HAL::Format::R16_FLOAT,             \
    HAL::Format::R16_UNORM,             \
    HAL::Format::R16_SNORM,             \
    HAL::Format::R8_UNORM,              \
    HAL::Format::R8_SNORM,              \
    HAL::Format::A8_UNORM,              \
    /*HAL::Format::R1_UNORM,*/          \
    HAL::Format::B5G6R5_UNORM,          \
    HAL::Format::B5G5R5A1_UNORM,        \
    HAL::Format::B8G8R8A8_UNORM,        \
    HAL::Format::B8G8R8X8_UNORM,        \
    HAL::Format::B8G8R8A8_UNORM_SRGB,   \
    HAL::Format::B8G8R8X8_UNORM_SRGB
#endif
