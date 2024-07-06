export module jinja2cpp;

export import "jinja2cpp.h";

// fix static variable linkage
namespace jinja2
{
	std::atomic_uint64_t UserCallable::m_gen{};

bool operator==(const Template& lhs, const Template& rhs)
{
    return lhs.IsEqual(rhs);
}

bool operator==(const TemplateW& lhs, const TemplateW& rhs)
{
    return lhs.IsEqual(rhs);
}

}
