export module stl.core;
export import <stl\core.h>;

export inline int current_errno() noexcept { return errno; }
