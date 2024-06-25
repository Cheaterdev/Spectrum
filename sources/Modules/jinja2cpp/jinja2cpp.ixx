export module jinja2cpp;

export import "jinja2cpp.h";

// fix static variable linkage
namespace jinja2
{
	std::atomic_uint64_t UserCallable::m_gen{};
}
