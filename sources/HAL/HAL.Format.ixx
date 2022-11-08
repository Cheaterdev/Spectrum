export module HAL:Format;
export import "Formats.h";

import Core;

export namespace HAL
{

	enum class NativeFormat : uint {};

	struct SurfaceInfo
	{
		uint numBytes;
		uint rowBytes;
		uint numRows;
	};

	class Format
	{
		Format(NativeFormat native_format) :native_format(Formats(static_cast<uint>(native_format))) {

		}
	public:
		enum Formats :uint
		{
			UNKNOWN,
			R32G32B32A32_TYPELESS,
			R32G32B32A32_FLOAT,
			R32G32B32A32_UINT,
			R32G32B32A32_SINT,
			R32G32B32_TYPELESS,
			R32G32B32_FLOAT,
			R32G32B32_UINT,
			R32G32B32_SINT,
			R16G16B16A16_TYPELESS,
			R16G16B16A16_FLOAT,
			R16G16B16A16_UNORM,
			R16G16B16A16_UINT,
			R16G16B16A16_SNORM,
			R16G16B16A16_SINT,
			R32G32_TYPELESS,
			R32G32_FLOAT,
			R32G32_UINT,
			R32G32_SINT,
			R32G8X24_TYPELESS,
			D32_FLOAT_S8X24_UINT,
			R32_FLOAT_X8X24_TYPELESS,
			X32_TYPELESS_G8X24_UINT,
			R10G10B10A2_TYPELESS,
			R10G10B10A2_UNORM,
			R10G10B10A2_UINT,
			R11G11B10_FLOAT,
			R8G8B8A8_TYPELESS,
			R8G8B8A8_UNORM,
			R8G8B8A8_UNORM_SRGB,
			R8G8B8A8_UINT,
			R8G8B8A8_SNORM,
			R8G8B8A8_SINT,
			R16G16_TYPELESS,
			R16G16_FLOAT,
			R16G16_UNORM,
			R16G16_UINT,
			R16G16_SNORM,
			R16G16_SINT,
			R32_TYPELESS,
			D32_FLOAT,
			R32_FLOAT,
			R32_UINT,
			R32_SINT,
			R24G8_TYPELESS,
			D24_UNORM_S8_UINT,
			R24_UNORM_X8_TYPELESS,
			X24_TYPELESS_G8_UINT,
			R8G8_TYPELESS,
			R8G8_UNORM,
			R8G8_UINT,
			R8G8_SNORM,
			R8G8_SINT,
			R16_TYPELESS,
			R16_FLOAT,
			D16_UNORM,
			R16_UNORM,
			R16_UINT,
			R16_SNORM,
			R16_SINT,
			R8_TYPELESS,
			R8_UNORM,
			R8_UINT,
			R8_SNORM,
			R8_SINT,
			A8_UNORM,
			R1_UNORM,
			R9G9B9E5_SHAREDEXP,
			R8G8_B8G8_UNORM,
			G8R8_G8B8_UNORM,
			BC1_TYPELESS,
			BC1_UNORM,
			BC1_UNORM_SRGB,
			BC2_TYPELESS,
			BC2_UNORM,
			BC2_UNORM_SRGB,
			BC3_TYPELESS,
			BC3_UNORM,
			BC3_UNORM_SRGB,
			BC4_TYPELESS,
			BC4_UNORM,
			BC4_SNORM,
			BC5_TYPELESS,
			BC5_UNORM,
			BC5_SNORM,
			B5G6R5_UNORM,
			B5G5R5A1_UNORM,
			B8G8R8A8_UNORM,
			B8G8R8X8_UNORM,
			R10G10B10_XR_BIAS_A2_UNORM,
			B8G8R8A8_TYPELESS,
			B8G8R8A8_UNORM_SRGB,
			B8G8R8X8_TYPELESS,
			B8G8R8X8_UNORM_SRGB,
			BC6H_TYPELESS,
			BC6H_UF16,
			BC6H_SF16,
			BC7_TYPELESS,
			BC7_UNORM,
			BC7_UNORM_SRGB,
		};

		using EnumClass = Formats;

		Format(Formats f) : native_format(f)
		{

		}
		Format() = default;

		operator Formats()
		{
			return native_format;
		}
		uint size() const;

		bool operator==(const Formats& v) const
		{

			return native_format == v;
		}
		bool operator==(const Format&) const = default;
		std::strong_ordering operator<=>(const Format& r)  const = default;

		uint get_native() const
		{
			return native_format;
		}

		bool is_shader_visible() const;
		bool is_srgb() const;
		Format to_typeless() const;

		Format to_dsv() const;
		Format to_srv() const;
		uint get_default_mapping() const;

		SurfaceInfo  surface_info(uint2 size)const;
	private:
		Formats native_format = Formats::UNKNOWN;

		SERIALIZE()
		{
			ar& NVP(native_format);
		}
	};

}
