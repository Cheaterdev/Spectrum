module;
#include <cerrno>

export module stl.core;
export import "core.h";

export inline int current_errno() noexcept { return errno; }
