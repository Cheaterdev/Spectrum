module;

#include "helper.h"

export module TextureData;

import FileSystem;
import serialization;
import Utils;

export
{

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
			ar& NVP(width)& NVP(height)& NVP(depth)& NVP(array_size)& NVP(mip_maps)& NVP(format);
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


		SERIALIZE()
		{

			ar& data;
			/*if constexpr (Archive::is_loading::value)
			{
				UINT size;
				ar& NVP(size);
				data.resize(size);
				ar.load_binary(data.data(), size);			
			}
			else
			{
				UINT size = static_cast<UINT>(data.size());
				ar& NVP(size);
				ar.save_binary(data.data(), size);
			}
			*/
			ar& NVP(width);
			ar& NVP(height);
			ar& NVP(depth);
			ar& NVP(width_stride);
			ar& NVP(slice_stride);
			ar& NVP(num_rows);
		}
		
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


		SERIALIZE()
		{
			SAVE_PARENT(texture_data_header);
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


}

module:private;

texture_data::ptr generate_tex_data(DirectX::ScratchImage& image)
{
	DirectX::TexMetadata metadata = image.GetMetadata();
	texture_data::ptr tex_data;

	// Fill out subresource array
	if (metadata.IsVolumemap())
	{
		size_t depth = metadata.depth;
		size_t idx = 0;
		tex_data.reset(new texture_data(1u, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, metadata.format));

		for (size_t level = 0; level < metadata.mipLevels; ++level)
		{
			size_t index = metadata.ComputeIndex(level, 0, 0);
			/*if (index >= nimages)
			return E_FAIL;*/
			const DirectX::Image& img = image.GetImages()[index];
			assert(idx < (metadata.mipLevels* metadata.arraySize));
			int  a = 0;
			size_t m = level;
			memcpy(tex_data->array[a]->mips[m]->data.data(), img.pixels, tex_data->array[a]->mips[m]->data.size());
			tex_data->array[a]->mips[m]->width_stride = static_cast<UINT>(img.rowPitch);
			tex_data->array[a]->mips[m]->slice_stride = static_cast<UINT>(img.slicePitch);
			/*             sub_data[idx].pData = img.pixels;
			sub_data[idx].RowPitch = static_cast<DWORD>(img.rowPitch);
			sub_data[idx].SlicePitch = static_cast<DWORD>(img.slicePitch);*/
			++idx;

			if (depth > 1)
				depth >>= 1;
		}
	}

	else
	{
		//--- 1D or 2D texture case ---------------------------------------------------
		size_t idx = 0;
		tex_data.reset(new texture_data((UINT)metadata.arraySize, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, metadata.format));

		for (size_t item = 0; item < metadata.arraySize; ++item)
		{
			for (size_t level = 0; level < metadata.mipLevels; ++level)
			{
				size_t index = metadata.ComputeIndex(level, item, 0);

				if (index >= image.GetImageCount())
					return nullptr;

				const DirectX::Image& img = image.GetImages()[index];

				if (img.format != metadata.format)
					return nullptr;

				if (!img.pixels)
					return nullptr;

				assert(idx < (metadata.mipLevels* metadata.arraySize));
				size_t  a = item;
				size_t m = level;
				memcpy(tex_data->array[a]->mips[m]->data.data(), img.pixels, tex_data->array[a]->mips[m]->data.size());
				tex_data->array[a]->mips[m]->width_stride = static_cast<UINT>(img.rowPitch);
				tex_data->array[a]->mips[m]->slice_stride = static_cast<UINT>(img.slicePitch);
				/*   sub_data[idx].pData = img.pixels;
				sub_data[idx].RowPitch = static_cast<DWORD>(img.rowPitch);
				sub_data[idx].SlicePitch = static_cast<DWORD>(img.slicePitch);*/
				++idx;
			}
		}
	}

	return tex_data;
	//  DirectX::CreateTextureEx(Device::get().get_native_device(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, header.force_srgb, (ID3D11Resource**)&native_texture);
}

texture_data::ptr texture_data::compress(texture_data::ptr orig)
{
	DirectX::ScratchImage compressed;
	DirectX::TexMetadata metadata;
	metadata.width = orig->width;
	metadata.height = orig->height;
	metadata.arraySize = orig->array_size;
	metadata.depth = orig->depth;
	metadata.format = orig->format;
	metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
	metadata.mipLevels = orig->mip_maps - 2;
	metadata.miscFlags = 0;
	metadata.miscFlags2 = 0;
	std::vector<DirectX::Image> images(metadata.mipLevels * metadata.arraySize);

	for (unsigned int i = 0; i < orig->array.size(); i++)
		for (unsigned int j = 0; j < metadata.mipLevels; j++)
		{
			auto& img = images[metadata.ComputeIndex(j, i, 0)];
			img.format = metadata.format;
			img.width = orig->array[i]->mips[j]->width;
			img.height = orig->array[i]->mips[j]->height;
			img.rowPitch = orig->array[i]->mips[j]->width_stride;
			img.slicePitch = orig->array[i]->mips[j]->slice_stride;
			img.pixels = reinterpret_cast<uint8_t*>(orig->array[i]->mips[j]->data.data());
		}
	assert(false);
	//  if (FAILED(DirectX::Compress(DX11::Device::get().get_native_device(), images.data(), images.size(), metadata, DXGI_FORMAT_BC7_UNORM_SRGB, 1, 1, compressed)))
	return nullptr;

	return generate_tex_data(compressed);
}

texture_data::ptr texture_data::load_texture(std::shared_ptr<file> file, int flags)
{
	bool mips = (flags & LoadFlags::GENERATE_MIPS);
	bool compress = (flags & LoadFlags::COMPRESS);

	if (file)
	{
		auto&& data = file->load_all();
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage orig_image;
		auto name = file->file_name;
		std::wstring ext = name.extension().generic_wstring();

		HRESULT hri = CoInitialize(NULL);

		HRESULT hr = NULL;
		if (ext == L".tga")
		{
			hr = DirectX::LoadFromTGAMemory(data.data(), data.size(), &metadata, orig_image);
		}

		else if (ext == L".dds")
		{
			hr = DirectX::LoadFromDDSMemory(data.data(), data.size(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, orig_image);
		}

		else 	hr = DirectX::LoadFromWICMemory(data.data(), data.size(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, orig_image);


		if (FAILED(hr))
			return nullptr;
		DirectX::ScratchImage mipChain;
		bool res = true;

		if (!mips || FAILED(DirectX::GenerateMipMaps(orig_image.GetImages(), orig_image.GetImageCount(), orig_image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
			res = false;

		DirectX::ScratchImage& image1 = res ? mipChain : orig_image;
		metadata = image1.GetMetadata();

		if (compress)
		{
			DirectX::ScratchImage compressed;
			res = true;
			metadata.mipLevels -= 2;

			assert(false);
			// if (FAILED(DirectX::Compress(DX11::Device::get().get_native_device(), image1.GetImages(), image1.GetImageCount() - 2, metadata, DXGI_FORMAT_BC7_UNORM_SRGB, 1, 1, compressed)))
			res = false;

			return generate_tex_data(res ? compressed : image1);
		}

		else
			return generate_tex_data(image1);
	}

	return nullptr;
}