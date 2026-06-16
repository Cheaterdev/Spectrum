// Non-module TU: wrapper functions that call DirectXTex APIs accepting DXGI_FORMAT.
// Module partitions cannot call these directly because d3d12.ixx exports
// `using DXGI_FORMAT = ::DXGI_FORMAT` which module-attaches the type, making it
// incompatible with the header-based DXGI_FORMAT that DirectXTex was compiled against.
// Defining wrappers here (plain .cpp, no module) uses the real header types,
// and the module side declares + calls them — the linker connects via mangled name.
//
// Add new wrappers here for any other DirectXTex API that takes DXGI_FORMAT
// (e.g. Convert, Compress) to restore features removed due to this limitation.

#include <directx/dxgiformat.h>
#include <DirectXTex.h>
#include <wincodec.h>
#pragma comment(lib, "ole32.lib")

#include <vector>
#include <cstdint>

// Expand R8_UNORM pixels to RGBA8 (g,g,g,255) and encode as PNG.
std::vector<uint8_t> hal_r8_to_png(const uint8_t* src, uint32_t width, uint32_t height)
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    DirectX::ScratchImage expanded;
    if (FAILED(expanded.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1)))
        return {};

    uint8_t* dst = expanded.GetImages()[0].pixels;
    for (uint32_t p = 0; p < width * height; ++p, ++src, dst += 4)
        { dst[0] = *src; dst[1] = *src; dst[2] = *src; dst[3] = 0xFF; }

    DirectX::Blob blob;
    if (FAILED(DirectX::SaveToWICMemory(*expanded.GetImages(),
            DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, blob)))
        return {};

    auto* bytes = static_cast<const uint8_t*>(blob.GetBufferPointer());
    return std::vector<uint8_t>(bytes, bytes + blob.GetBufferSize());
}
