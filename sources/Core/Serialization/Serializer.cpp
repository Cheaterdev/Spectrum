#include "pch.h"
#include "Utils/Hash/crc32.h"

std::string Hasher::hash(std::string_view str)
{
	return to_string(crc32(str));
}
