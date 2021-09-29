#include "Serialization/serialization.h"
#include "FileSystem/FileSystem.h"

#include "helper.h"
static void GetSurfaceInfo2(_In_ size_t width,
                            _In_ size_t height,
                            _In_ DXGI_FORMAT fmt,
                            _Out_opt_ size_t* outNumBytes,
                            _Out_opt_ size_t* outRowBytes,
                            _Out_opt_ size_t* outNumRows)
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;
    bool bc = false;
    bool packed = false;
    size_t bcnumBytesPerBlock = 0;

    switch (fmt)
    {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bcnumBytesPerBlock = 8;
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bcnumBytesPerBlock = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            packed = true;
            break;
    }

    if (bc)
    {
        size_t numBlocksWide = 0;

        if (width > 0)
            numBlocksWide = std::max<size_t>(1, (width + 3) / 4);

        size_t numBlocksHigh = 0;

        if (height > 0)
            numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);

        rowBytes = numBlocksWide * bcnumBytesPerBlock;
        numRows = numBlocksHigh;
    }

    else if (packed)
    {
        rowBytes = ((width + 1) >> 1) * 4;
        numRows = height;
    }

    else
    {
        size_t bpp = BitsPerPixel(fmt);
        rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
        numRows = height;
    }

    numBytes = rowBytes * numRows;

    if (outNumBytes)
        *outNumBytes = numBytes;

    if (outRowBytes)
        *outRowBytes = rowBytes;

    if (outNumRows)
        *outNumRows = numRows;
}

struct texture_data_header
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    uint32_t array_size;
    uint32_t mip_maps;
    DXGI_FORMAT format;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar& NVP(width) &NVP(height) &NVP(depth) &NVP(array_size) &NVP(mip_maps)&NVP(format);
    }
};


struct texture_mip_data
{

    using ptr = std::shared_ptr<texture_mip_data>;
    texture_mip_data() {}
    texture_mip_data(UINT w, UINT h, UINT d, DXGI_FORMAT format)
    {
        width = w;
        height = h;
        depth = d;
        size_t a, b, c;
        GetSurfaceInfo2(w, h, format, &a, &b, &c);
        width_stride = static_cast<UINT>(b);
        slice_stride = static_cast<UINT>(a);
        num_rows = static_cast<UINT>(c);
        data.resize(slice_stride * d);
    }
    std::vector<unsigned char> data;
	UINT width;
	UINT height;
	UINT depth;
	UINT num_rows;
	UINT width_stride;
	UINT slice_stride;


    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive& ar, const unsigned int) const
    {
		UINT size = static_cast<UINT>(data.size());
        ar& NVP(size);
        ar.save_binary(data.data(), size);
        ar& NVP(width);
        ar& NVP(height);
        ar& NVP(depth);
        ar& NVP(width_stride);
        ar& NVP(slice_stride);
        ar& NVP(num_rows);
    }
    template<class Archive>
    void load(Archive& ar, const unsigned int)
    {
		UINT size;
        ar& NVP(size);
        data.resize(size);
        ar.load_binary(data.data(), size);
        ar& NVP(width);
        ar& NVP(height);
        ar& NVP(depth);
        ar& NVP(width_stride);
        ar& NVP(slice_stride);
        ar& NVP(num_rows);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

struct mip
{
    using ptr = std::shared_ptr<mip>;

    std::vector<texture_mip_data::ptr> mips;
    mip() {}
    mip(uint32_t count, uint32_t width, uint32_t height, uint32_t depth, DXGI_FORMAT format)
    {
        mips.reserve(count);

        for (uint32_t i = 0; i < count; i++)
        {
            mips.emplace_back(std::make_shared<texture_mip_data>(width, height, depth, format));
            width /= 2;
            height /= 2;
            depth /= 2;

            if (width < 1) width = 1;

            if (height < 1) height = 1;

            if (depth < 1) depth = 1;
        }
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar& NVP(mips);
    }
};

class texture_data : public texture_data_header
{
    public:
        using ptr = std::shared_ptr<texture_data>;


        std::vector<mip::ptr> array;



        enum  LoadFlags
        {
            MAKE_LINEAR = 1,
            GENERATE_MIPS = 2,
            COMPRESS = 4
        };


        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<texture_data_header>(*this));
            ar& NVP(array);
        }
        texture_data() {}
        texture_data(uint32_t array_count, uint32_t num_mips, uint32_t width, uint32_t height, uint32_t depth, DXGI_FORMAT format)
        {
            array_size = array_count;
            this->depth = depth;
            this->format = format;
            this->height = height;
            this->mip_maps = num_mips;
            this->width = width;
            array.reserve(array_count);

            for (uint32_t i = 0; i < array_count; i++)
                array.emplace_back(std::make_shared<mip>(num_mips, width, height, depth, format));
        }

        static std::unique_ptr<DirectX::TexMetadata> get_metadata(std::shared_ptr<file> file)
        {
			auto name = file->file_name.generic_wstring();
			auto ext = file->file_name.extension().generic_wstring();
          //  std::wstring ext = to_lower(name.substr(name.find_last_of(L".") + 1));
            DirectX::TexMetadata metadata;

            if (ext == L"tga")
            {
                if (SUCCEEDED(DirectX::GetMetadataFromTGAFile(name.c_str(), metadata)))
                    return std::make_unique<DirectX::TexMetadata>(metadata);
            }

            else if (SUCCEEDED(DirectX::GetMetadataFromWICFile(name.c_str(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, metadata)))
                return std::make_unique<DirectX::TexMetadata>(metadata);

            return nullptr;
        }


        static ptr load_texture(std::shared_ptr<file> file, int flags);
        static ptr compress(ptr orig);

};