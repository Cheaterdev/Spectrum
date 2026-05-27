module Core:crc32;

import crc32c;
import stl.core;
import :Utils;

unsigned int crc32(const binary& s)
{
	return crc32c::Crc32c((const uint8_t*)s.data(), s.size());
}

unsigned int crc32(std::string_view s)
{
	return crc32c::Crc32c(s);
}
