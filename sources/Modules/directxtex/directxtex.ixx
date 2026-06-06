module;

// Minimal Windows includes for HRESULT, DXGI_FORMAT, and stdint types.
// WIN32_LEAN_AND_MEAN and NOMINMAX are set globally by the build system.
#include <windows.h>
#include <directx/dxgiformat.h>

export module directxtex;

export namespace DirectXTex
{

    // -------------------------------------------------------------------------
    // Dimension enum (mirrors DirectX::TEX_DIMENSION)
    // -------------------------------------------------------------------------
    enum TEX_DIMENSION
    {
        TEX_DIMENSION_TEXTURE1D = 2,
        TEX_DIMENSION_TEXTURE2D = 3,
        TEX_DIMENSION_TEXTURE3D = 4,
    };

    // -------------------------------------------------------------------------
    // DDS_FLAGS (mirrors DirectX::DDS_FLAGS)
    // -------------------------------------------------------------------------
    enum DDS_FLAGS : unsigned long
    {
        DDS_FLAGS_NONE                 = 0x0,
        DDS_FLAGS_LEGACY_DWORD         = 0x1,
        DDS_FLAGS_NO_LEGACY_EXPANSION  = 0x2,
        DDS_FLAGS_NO_R10B10G10A2_FIXUP = 0x4,
        DDS_FLAGS_FORCE_RGB            = 0x8,
        DDS_FLAGS_NO_16BPP             = 0x10,
        DDS_FLAGS_EXPAND_LUMINANCE     = 0x20,
        DDS_FLAGS_BAD_DXTN_TAILS       = 0x40,
        DDS_FLAGS_PERMISSIVE           = 0x80,
        DDS_FLAGS_IGNORE_MIPS          = 0x100,
        DDS_FLAGS_FORCE_DX10_EXT       = 0x10000,
        DDS_FLAGS_FORCE_DX10_EXT_MISC2 = 0x20000,
        DDS_FLAGS_FORCE_DX9_LEGACY     = 0x40000,
        DDS_FLAGS_FORCE_DXT5_RXGB      = 0x80000,
        DDS_FLAGS_ALLOW_LARGE_FILES    = 0x1000000,
    };

    // -------------------------------------------------------------------------
    // WIC_FLAGS (mirrors DirectX::WIC_FLAGS)
    // -------------------------------------------------------------------------
    enum WIC_FLAGS : unsigned long
    {
        WIC_FLAGS_NONE             = 0x0,
        WIC_FLAGS_FORCE_RGB        = 0x1,
        WIC_FLAGS_NO_X2_BIAS       = 0x2,
        WIC_FLAGS_NO_16BPP         = 0x4,
        WIC_FLAGS_ALLOW_MONO       = 0x8,
        WIC_FLAGS_ALL_FRAMES       = 0x10,
        WIC_FLAGS_IGNORE_SRGB      = 0x20,
        WIC_FLAGS_FORCE_SRGB       = 0x40,
        WIC_FLAGS_FORCE_LINEAR     = 0x80,
        WIC_FLAGS_DEFAULT_SRGB     = 0x100,
        WIC_FLAGS_DITHER           = 0x10000,
        WIC_FLAGS_DITHER_DIFFUSION = 0x20000,
        WIC_FLAGS_FILTER_POINT     = 0x100000,
        WIC_FLAGS_FILTER_LINEAR    = 0x200000,
        WIC_FLAGS_FILTER_CUBIC     = 0x300000,
        WIC_FLAGS_FILTER_FANT      = 0x400000,
    };

    // -------------------------------------------------------------------------
    // TEX_FILTER_FLAGS (mirrors DirectX::TEX_FILTER_FLAGS)
    // -------------------------------------------------------------------------
    enum TEX_FILTER_FLAGS : unsigned long
    {
        TEX_FILTER_DEFAULT          = 0,
        TEX_FILTER_WRAP_U           = 0x1,
        TEX_FILTER_WRAP_V           = 0x2,
        TEX_FILTER_WRAP_W           = 0x4,
        TEX_FILTER_WRAP             = 0x7,
        TEX_FILTER_MIRROR_U         = 0x10,
        TEX_FILTER_MIRROR_V         = 0x20,
        TEX_FILTER_MIRROR_W         = 0x40,
        TEX_FILTER_MIRROR           = 0x70,
        TEX_FILTER_SEPARATE_ALPHA   = 0x100,
        TEX_FILTER_FLOAT_X2BIAS     = 0x200,
        TEX_FILTER_RGB_COPY_RED     = 0x1000,
        TEX_FILTER_RGB_COPY_GREEN   = 0x2000,
        TEX_FILTER_RGB_COPY_BLUE    = 0x4000,
        TEX_FILTER_RGB_COPY_ALPHA   = 0x8000,
        TEX_FILTER_DITHER           = 0x10000,
        TEX_FILTER_DITHER_DIFFUSION = 0x20000,
        TEX_FILTER_POINT            = 0x100000,
        TEX_FILTER_LINEAR           = 0x200000,
        TEX_FILTER_CUBIC            = 0x300000,
        TEX_FILTER_BOX              = 0x400000,
        TEX_FILTER_FANT             = 0x400000,
        TEX_FILTER_TRIANGLE         = 0x500000,
        TEX_FILTER_SRGB_IN          = 0x1000000,
        TEX_FILTER_SRGB_OUT         = 0x2000000,
        TEX_FILTER_SRGB             = 0x3000000,
        TEX_FILTER_FORCE_NON_WIC    = 0x10000000,
        TEX_FILTER_FORCE_WIC        = 0x20000000,
    };

    // -------------------------------------------------------------------------
    // TEX_COMPRESS_FLAGS (mirrors DirectX::TEX_COMPRESS_FLAGS)
    // -------------------------------------------------------------------------
    enum TEX_COMPRESS_FLAGS : unsigned long
    {
        TEX_COMPRESS_DEFAULT          = 0,
        TEX_COMPRESS_RGB_DITHER       = 0x10000,
        TEX_COMPRESS_A_DITHER         = 0x20000,
        TEX_COMPRESS_DITHER           = 0x30000,
        TEX_COMPRESS_UNIFORM          = 0x40000,
        TEX_COMPRESS_BC7_USE_3SUBSETS = 0x80000,
        TEX_COMPRESS_BC7_QUICK        = 0x100000,
        TEX_COMPRESS_SRGB_IN          = 0x1000000,
        TEX_COMPRESS_SRGB_OUT         = 0x2000000,
        TEX_COMPRESS_SRGB             = 0x3000000,
        TEX_COMPRESS_PARALLEL         = 0x10000000,
    };

    // -------------------------------------------------------------------------
    // Image — same layout as DirectX::Image
    // -------------------------------------------------------------------------
    struct Image
    {
        size_t      width;
        size_t      height;
        DXGI_FORMAT format;
        size_t      rowPitch;
        size_t      slicePitch;
        uint8_t*    pixels;
    };

    // -------------------------------------------------------------------------
    // TexMetadata — same layout as DirectX::TexMetadata
    // -------------------------------------------------------------------------
    struct TexMetadata
    {
        size_t        width;
        size_t        height;
        size_t        depth;
        size_t        arraySize;
        size_t        mipLevels;
        uint32_t      miscFlags;
        uint32_t      miscFlags2;
        DXGI_FORMAT   format;
        TEX_DIMENSION dimension;

        bool IsVolumemap() const noexcept { return dimension == TEX_DIMENSION_TEXTURE3D; }

        // Non-inline — defined in directxtex.cpp (module implementation unit).
        // This ensures the symbol is compiled in the directxtex module context,
        // generating DirectXTex::TexMetadata::ComputeIndex::<!directxtex> which
        // matches the call-site decoration added by MSVC for module-imported types.
        size_t ComputeIndex(size_t mip, size_t item, size_t slice) const noexcept;
    };

    // -------------------------------------------------------------------------
    // ScratchImage — PIMPL wrapper around DirectX::ScratchImage.
    //
    // All non-trivial methods are defined in directxtex.cpp so that the
    // module-context decorated symbols (::<!directxtex>) are generated there
    // and match what callers that `import directxtex` expect.
    // -------------------------------------------------------------------------
    class ScratchImage
    {
        void* impl_; // heap-allocated ::DirectX::ScratchImage
    public:
        ScratchImage() noexcept;
        ScratchImage(ScratchImage&&) noexcept;
        ScratchImage& operator=(ScratchImage&&) noexcept;
        ScratchImage(const ScratchImage&)            = delete;
        ScratchImage& operator=(const ScratchImage&) = delete;
        ~ScratchImage();

        const TexMetadata& GetMetadata()   const noexcept;
        const Image*       GetImages()     const noexcept;
        size_t             GetImageCount() const noexcept;
        void               Release()       noexcept;

        // Friends declared here so the free-function implementations
        // in directxtex.cpp can access impl_.
        friend HRESULT GetMetadataFromTGAFile(const wchar_t*, TexMetadata&) noexcept;
        friend HRESULT GetMetadataFromWICFile(const wchar_t*, WIC_FLAGS, TexMetadata&) noexcept;
        friend HRESULT LoadFromTGAMemory(const void*, size_t, TexMetadata*, ScratchImage&) noexcept;
        friend HRESULT LoadFromDDSMemory(const void*, size_t, DDS_FLAGS, TexMetadata*, ScratchImage&) noexcept;
        friend HRESULT LoadFromWICMemory(const void*, size_t, WIC_FLAGS, TexMetadata*, ScratchImage&) noexcept;
        friend HRESULT GenerateMipMaps(const Image*, size_t, const TexMetadata&, TEX_FILTER_FLAGS, size_t, ScratchImage&) noexcept;
        friend HRESULT Compress(const Image*, size_t, const TexMetadata&, DXGI_FORMAT, TEX_COMPRESS_FLAGS, float, ScratchImage&) noexcept;
    };

    // -------------------------------------------------------------------------
    // Free functions (implementations in directxtex.cpp)
    // -------------------------------------------------------------------------
    HRESULT GetMetadataFromTGAFile(const wchar_t* szFile, TexMetadata& metadata) noexcept;
    HRESULT GetMetadataFromWICFile(const wchar_t* szFile, WIC_FLAGS flags, TexMetadata& metadata) noexcept;
    HRESULT LoadFromTGAMemory(const void* pSource, size_t size, TexMetadata* metadata, ScratchImage& image) noexcept;
    HRESULT LoadFromDDSMemory(const void* pSource, size_t size, DDS_FLAGS flags, TexMetadata* metadata, ScratchImage& image) noexcept;
    HRESULT LoadFromWICMemory(const void* pSource, size_t size, WIC_FLAGS flags, TexMetadata* metadata, ScratchImage& image) noexcept;
    HRESULT GenerateMipMaps(const Image* srcImages, size_t nimages, const TexMetadata& metadata, TEX_FILTER_FLAGS filter, size_t levels, ScratchImage& mipChain) noexcept;
    HRESULT Compress(const Image* srcImages, size_t nimages, const TexMetadata& metadata, DXGI_FORMAT format, TEX_COMPRESS_FLAGS compress, float threshold, ScratchImage& cImage) noexcept;

}
