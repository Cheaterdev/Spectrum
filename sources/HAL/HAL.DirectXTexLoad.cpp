// Non-module TU: texture file decoding through DirectXTex, shared by both
// backends' texture_data::load_from_memory -- TGA/DDS/WIC by extension,
// GenerateMipMaps on request, the block-compressed base-size clamp -- so
// D3D12 and Vulkan hand the GPU the same texels and mips.
//
// A plain .cpp because DirectXTex's DXGI_FORMAT is the header type; the module
// side declares hal_dxtex_load with standard types only (a struct declared in
// a module is module-attached and would not link against one declared here).
// HAL::Format numbers formats as DXGI does, so desc[0] converts with a cast.

#include <DirectXTex.h>
#include <objbase.h>
#pragma comment(lib, "ole32.lib")

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// desc: { dxgi_format, width, height, depth, array_size, mip_levels }.
// Subresources item-major then mip; a volume mip holds all its depth slices.
bool hal_dxtex_load(const void* data, size_t size, std::string ext, bool generate_mips,
                    std::vector<uint32_t>& desc,
                    std::vector<std::vector<uint8_t>>& subresources,
                    std::vector<uint32_t>& row_pitches,
                    std::vector<uint32_t>& slice_pitches)
{
    if (!ext.empty() && ext[0] == '.')
        ext.erase(0, 1);
    for (auto& c : ext)
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    DirectX::TexMetadata  metadata;
    DirectX::ScratchImage loaded;
    auto bytes = static_cast<const uint8_t*>(data);
    HRESULT hr;
    if (ext == "tga")
        hr = DirectX::LoadFromTGAMemory(bytes, size, &metadata, loaded);
    else if (ext == "dds")
        hr = DirectX::LoadFromDDSMemory(bytes, size, DirectX::DDS_FLAGS_NONE, &metadata, loaded);
    else
        hr = DirectX::LoadFromWICMemory(bytes, size, DirectX::WIC_FLAGS_NONE, &metadata, loaded);
    if (FAILED(hr))
        return false;

    DirectX::ScratchImage mip_chain;
    const bool mipped = generate_mips && SUCCEEDED(DirectX::GenerateMipMaps(loaded.GetImages(), loaded.GetImageCount(),
        loaded.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mip_chain));
    const DirectX::ScratchImage& image = mipped ? mip_chain : loaded;
    metadata = image.GetMetadata();

    // Same clamp as D3D12's generate_tex_data: a block-compressed base below
    // 4x4 is stored as one full block, and mips stop at the last full block.
    if (DirectX::IsCompressed(metadata.format))
    {
        if (metadata.width < 4)  metadata.width  = 4;
        if (metadata.height < 4) metadata.height = 4;

        size_t max_mips = 1;
        while ((metadata.width >> max_mips) >= 4 && (metadata.height >> max_mips) >= 4)
            max_mips++;
        if (metadata.mipLevels > max_mips)
            metadata.mipLevels = max_mips;
    }

    const bool   volume     = metadata.IsVolumemap();
    const size_t array_size = volume ? 1 : metadata.arraySize;
    desc = { uint32_t(metadata.format), uint32_t(metadata.width), uint32_t(metadata.height),
             uint32_t(metadata.depth), uint32_t(array_size), uint32_t(metadata.mipLevels) };

    subresources.clear();
    row_pitches.clear();
    slice_pitches.clear();
    for (size_t item = 0; item < array_size; ++item)
    {
        size_t depth = metadata.depth;
        for (size_t level = 0; level < metadata.mipLevels; ++level)
        {
            // The image's own metadata: the clamp above may have dropped mips,
            // which would shift every later array item's index.
            const size_t index = image.GetMetadata().ComputeIndex(level, item, 0);
            if (index >= image.GetImageCount())
                return false;
            const DirectX::Image& img = image.GetImages()[index];
            if (img.format != metadata.format || !img.pixels)
                return false;

            // A volume mip's slices are consecutive images in one allocation.
            const size_t slices = volume ? depth : 1;
            subresources.emplace_back(img.pixels, img.pixels + img.slicePitch * slices);
            row_pitches.push_back(uint32_t(img.rowPitch));
            slice_pitches.push_back(uint32_t(img.slicePitch));

            if (depth > 1)
                depth >>= 1;
        }
    }
    return true;
}
