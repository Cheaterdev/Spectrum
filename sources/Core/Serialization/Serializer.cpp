#include "pch.h"

#include "Serialization/Serializer.h"

import Utils;
import crc32;

std::string Hasher::hash(std::string_view str)
{
	return to_string(crc32(str));
}
