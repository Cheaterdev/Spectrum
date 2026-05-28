module HAL:Format;
import "Formats.h";
import Core;

namespace HAL
{
	Format::Format(NativeFormat native_format) : native_format(Formats(static_cast<uint>(native_format))) {}

	Format::Format(Formats f) : native_format(f) {}

	Format::operator Formats() { return native_format; }

	bool Format::operator==(const Formats& v) const { return native_format == v; }

	bool Format::operator==(const Format& r) const { return native_format == r.native_format; }

	uint Format::get_native() const { return native_format; }

	std::string Format::to_string() { return std::string(magic_enum::enum_name(native_format)); }
}
