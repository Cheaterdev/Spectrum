#pragma once
#include <span>

namespace Passes
{

class PassNodeBase
{
public:
	virtual ~PassNodeBase() = default;
	virtual std::span<const wchar_t* const> GetUsedResourcesList() const = 0;
};

}
