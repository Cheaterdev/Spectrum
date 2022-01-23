export module HAL.Format;

import Vectors;
import Utils;
import serialization;

export namespace HAL
{

	class Format
	{
	public:

		Format() = default;
		Format(explicit uint native_format):native_format(native_format) {

		}

		uint size();

		bool operator==(const Format&) const = default;
		std::strong_ordering operator<=>(const Format& r)  const = default;

		uint get_native() const
		{
			return native_format;
		}

		bool is_shader_visible() const;

		Format to_typeless() const;
	private:
		uint native_format = 0;

		SERIALIZE()
		{
			ar& NVP(native_format);
		}
	};


	struct Formats
	{
		static const Format Unknown;


		static const Format R8G8B8A8_UNORM_SRGB;
		static const Format R8G8B8A8_UNORM;
		static const Format R8G8B8A8_UINT;
		static const Format R8G8B8A8_SNORM;
		static const Format R8G8B8A8_SINT;

		static const Format R8G8B8A8_TYPELESS;

	};
}
