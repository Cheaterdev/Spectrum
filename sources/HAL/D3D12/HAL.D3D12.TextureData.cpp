module HAL:TextureData;

import Core;
import d3d12;

import :Utils;
import :Types;



namespace HAL
{

	std::unique_ptr<DirectX::TexMetadata> get_metadata(std::shared_ptr<file> file)
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




	texture_mip_data::texture_mip_data(UINT w, UINT h, UINT d, Format format)
	{
		width = w;
		height = h;
		depth = d;
		auto info = format.surface_info({w,h});
		width_stride = info.rowBytes;
		slice_stride = info.numBytes;
		num_rows = info.numRows;
		data.resize(slice_stride * d);
	}

		mip::mip(uint32_t count, uint32_t width, uint32_t height, uint32_t depth, Format format)
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


	texture_data::texture_data(uint32_t array_count, uint32_t num_mips, uint32_t width, uint32_t height, uint32_t depth, Format format)
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



}



namespace HAL
{

	texture_data::ptr generate_tex_data(DirectX::ScratchImage& image)
	{
		DirectX::TexMetadata metadata = image.GetMetadata();
		texture_data::ptr tex_data;

		// Fill out subresource array
		if (metadata.IsVolumemap())
		{
			size_t depth = metadata.depth;
			size_t idx = 0;
			tex_data.reset(new texture_data(1u, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, from_native(metadata.format)));

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
			tex_data.reset(new texture_data((UINT)metadata.arraySize, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, from_native(metadata.format)));

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
		metadata.format = to_native(orig->format);
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


}