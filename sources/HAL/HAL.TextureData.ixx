export module HAL:TextureData;


import Core;

import :Utils;
import :Types;

export namespace HAL
{

	class texture_data_header
	{
	public:
		uint width = 0;
		uint height = 0;
		uint depth = 0;

		uint array_size = 0;
		uint mip_maps = 0;
		Format format;

		virtual~texture_data_header() = default;

	private:
		SERIALIZE()
		{
			ar& NVP(width);
			ar& NVP(height);
			ar& NVP(depth);
			ar& NVP(array_size);
			ar& NVP(mip_maps);
			ar& NVP(format);
		}
	};


	struct texture_mip_data
	{

		using ptr = std::shared_ptr<texture_mip_data>;
		texture_mip_data() {}
		texture_mip_data(uint w, uint h, uint d, Format format);
		std::vector<unsigned char> data;
		uint width;
		uint height;
		uint depth;
		uint num_rows;
		uint width_stride;
		uint slice_stride;


		SERIALIZE()
		{
			ar& NVP(data);
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
		mip() = default;
		mip(uint count, uint width, uint height, uint depth, Format format);

		SERIALIZE()
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
		texture_data() = default;
		texture_data(uint array_count, uint num_mips, uint width, uint height, uint depth, Format format);

		static ptr load_texture(std::shared_ptr<file> file, int flags);

		// Decode an in-memory image blob (PNG/JPG/TGA/DDS/...).
		// format_hint is an extension with or without the leading dot ("png", ".dds");
		// empty means auto-detect via WIC.
		static ptr load_from_memory(const void* data, size_t size, std::string format_hint, int flags);

		static ptr compress(ptr orig);

		// Build from GPU readback data: strips row padding (layout.row_stride → width_stride)
		static ptr from_readback(uint width, uint height, Format fmt,
		                         std::span<const std::byte> gpu_data,
		                         const texture_layout& layout);

		// Encode mip[0] as PNG bytes (converts to R8G8B8A8_UNORM if needed)
		std::vector<uint8_t> to_png() const;

		// Decode PNG bytes into texture_data
		static ptr from_png(const void* data, size_t size);

	};


}

//module:private;

REGISTER_TYPE(HAL::texture_data);
CEREAL_FORCE_REGISTER(HAL::texture_data);