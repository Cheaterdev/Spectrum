#pragma once
// magic_enum wrapper.
// Range configuration is local to this wrapper — magic_enum needs these set
// before its headers are parsed into the IFC.
#ifndef MAGIC_ENUM_RANGE_MIN
#  define MAGIC_ENUM_RANGE_MIN 0
#  define MAGIC_ENUM_RANGE_MAX 1024
#endif
#include <magic_enum/magic_enum_all.hpp>
